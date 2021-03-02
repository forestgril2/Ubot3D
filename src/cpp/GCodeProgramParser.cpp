#include "GCodeProgramParser.h"

#include <gcode_program.h>
#include <parser.h>

static gpr::gcode_program importGCodeFromFile(const std::string& file)
{
	Chronograph chronograph("Read gcode file contents", true);
	std::ifstream t(file);
	std::string file_contents((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	chronograph.log();

	chronograph.start("Parse gcode");
	return gpr::parse_gcode(file_contents);
}

ExtruderData GCodeProgramParser::createExtruderData(const std::string& inputFile)
{
	Chronograph chronograph(__FUNCTION__, true);

	gpr::gcode_program gcodeProgram = importGCodeFromFile(inputFile);

	ExtruderData data;
	ExtrPath subPath;

	std::vector<ExtrPath>& extruderPaths = data.extruderPaths;
	extruderPaths.push_back(ExtrPath());

	std::vector<std::pair<uint32_t, float>>& layerBottoms = data.layerBottoms;
	// Layer bottom at start is just the bed level.
	layerBottoms.push_back({0u, 0.0f});

	unsigned maxPointsInSubPath = 0;

	bool isExtruderOn = false;
	bool isAbsoluteMode = true;
	unsigned blockCount = 0;
	const unsigned blockCountLimit = 5220800;
	std::string blockString;

	ExtrPoint lastAbsCoords(0,0,0,0);
	ExtrPoint blockAbsCoords(0,0,0,0);
	ExtrPoint* newCoordsPtr = &blockAbsCoords;
	for (auto it = gcodeProgram.begin();
		 it != gcodeProgram.end() && blockCount < blockCountLimit;
		 ++it, ++blockCount)
	{
		const auto& block = *it;
		blockString = block.to_string();
		ExtrPoint blockRelativeCoords(0,0,0,0);

		auto setExtrusionOff = [&extruderPaths, &maxPointsInSubPath, &blockString, &subPath, &isExtruderOn]()
		{// If we are setting extrusion off, swap created path with the empty one in path vector.
			if (!isExtruderOn)
				return;
			isExtruderOn = false;

			if (subPath.empty())
				return;
			maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());

			dumpSubPath(blockString, subPath);

			if (subPath.size() < 2)
			{
				static bool wasSubpathOfSmallSizeAnounced = false;
				if (!wasSubpathOfSmallSizeAnounced)
				{
					std::cout << "### WARNING: subpath of size: " << subPath.size() << std::endl;
					wasSubpathOfSmallSizeAnounced = true;
				}

				subPath.clear();
				return;
			}

			std::swap(extruderPaths.back(), subPath);
			extruderPaths.push_back(ExtrPath());
		};

		auto setExtrusionOn = [&subPath, &isExtruderOn](const ExtrPoint& lastAbsCoords)
		{// If we are setting extrusion on, add new (empty) path to path vector.
			if (isExtruderOn)
				return;
			isExtruderOn = true;
			subPath.push_back(lastAbsCoords);
		};

		auto setAbsoluteModeOn = [&isAbsoluteMode, &newCoordsPtr, &blockAbsCoords]() {
			if (isAbsoluteMode)
				return;
			isAbsoluteMode = true;
			newCoordsPtr = &blockAbsCoords;
		};

		auto setAbsoluteModeOff = [&isAbsoluteMode, &newCoordsPtr, &blockRelativeCoords]() {
			if (!isAbsoluteMode)
				return;

			isAbsoluteMode = false;
			newCoordsPtr = &blockRelativeCoords;
		};

		Vector4i whichCoordsSetInBlock(0,0,0,0);

		for (const gpr::chunk& chunk : block)
		{
			switch(chunk.tp())
			{
				case gpr::CHUNK_TYPE_PERCENT:
				case gpr::CHUNK_TYPE_WORD:
					break;

				case gpr::CHUNK_TYPE_COMMENT:
				{
					const std::string comment = chunk.get_comment_text();
					if (comment._Starts_with("LAYER:") && (0 != comment.compare("LAYER:0")))
					{// First layer has bed level (0.0f), like all preparation steps. We enter here only for layers 1+
						const ExtrPath& previousSubPath = extruderPaths[extruderPaths.size() -2];
						const ExtrPoint& previousLayerLastPoint = previousSubPath[previousSubPath.size() -1];
						layerBottoms.push_back({extruderPaths.size(), previousLayerLastPoint.z()});
					}
					break;
				}
				case gpr::CHUNK_TYPE_WORD_ADDRESS:
				{
					const gpr::addr& adddres = chunk.get_address();
					const char& word = chunk.get_word();

					switch(word)
					{
						// Header movements.
						case 'G':
							switch (adddres.tp())
							{
								case gpr::ADDRESS_TYPE_INTEGER:
									switch (adddres.int_value())
									{
										case 0:
											setExtrusionOff();
											break;
										case 1:
											setExtrusionOn(lastAbsCoords);
											break;
										case 90:
											setAbsoluteModeOn();
											break;
										case 91:
											setAbsoluteModeOff();
											break;
										case 92:
											//TODO: IMPLEMENT.
											break;
										default:
											break;
									}
									break;

								case gpr::ADDRESS_TYPE_DOUBLE:
									assert(false);
									break;
							}
							break;

						// Path point coordinates.
						case 'X':
							newCoordsPtr->x() = float(adddres.double_value());
							whichCoordsSetInBlock.x() = true;
							break;
						case 'Y':
							newCoordsPtr->y() = float(adddres.double_value());
							whichCoordsSetInBlock.y() = true;
							break;
						case 'Z':
							newCoordsPtr->z() = float(adddres.double_value());
							whichCoordsSetInBlock.z() = true;
							break;

						// Extruded length.
						case 'E':
							newCoordsPtr->w() = float(adddres.double_value());
							whichCoordsSetInBlock.w() = true;
							break;

						default:
							break;
					}
					break;
				}
			}
		}

		if (whichCoordsSetInBlock.isZero())
			continue;

		for (unsigned short i = 0; i < whichCoordsSetInBlock.size(); ++i)
		{// updateBlockCoords(): If this coord is not set in this block, use the most recent for absolute coordinates.
			if (whichCoordsSetInBlock[i] != 0 || !isAbsoluteMode)
				continue;
			blockAbsCoords[i] = lastAbsCoords[i];
		}

		if (!isAbsoluteMode)
		{
			lastAbsCoords += blockRelativeCoords;
		}
		else
		{
			lastAbsCoords = blockAbsCoords;
		}

		if (isExtruderOn && (subPath.size() == 0 || lastAbsCoords != subPath.back()))
		{// Ignore movements without extrusion and identical extruder path coordinates.
			subPath.push_back(lastAbsCoords);
		}
	}

	if (!subPath.empty())
	{
		maxPointsInSubPath = std::max<unsigned>(maxPointsInSubPath, subPath.size());
		std::cout << " #### adding subPath no. " << extruderPaths.size() -1<< ", maxPointsInSubPath: " << maxPointsInSubPath << std::endl;

		dumpSubPath(blockString, subPath);

		std::swap(extruderPaths.back(), subPath);
	}

	data.maxNumPointsInSubPath = maxPointsInSubPath;
	data.numSubPaths = extruderPaths.size();

	return data;
}

void GCodeProgramParser::dumpSubPath(const std::string& blockString, const ExtrPath& subPath)
{
	std::ofstream pathFile("subPathDump.txt");
	for (const ExtrPoint& point : subPath)
	{
		pathFile << "[" << point.x() << "," << point.y() << "," << point.z() << "," << point.w() << "]" << std::endl;
	}
	pathFile << blockString << std::endl;
	pathFile.close();
}

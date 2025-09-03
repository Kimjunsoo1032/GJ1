#pragma once
#include "KamataEngine.h"

enum class MapChipType {
	kBlank,
	kBlock,
	kPlatform,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {

	MapChipData mapChipData_;


public:
	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};
	MapChipField::IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type);
	struct Rect {
		float right;
		float left;
		float top;
		float bottom;

	};
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);


private:
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	
};

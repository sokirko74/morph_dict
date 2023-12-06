#pragma once 

struct CFuzzyResult {
	std::string CorrectedString;
	int StringDistance;
	bool operator < (const  CFuzzyResult& x) const {
		return StringDistance < x.StringDistance;
	}
};

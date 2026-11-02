#ifndef LEVELLBACKGROUNDPARK_HH
#define LEVELLBACKGROUNDPARK_HH

#include "LevelBackground.hh"

class LevelBackgroundPark : public LevelBackground {
public:
	LevelBackgroundPark(const int width, const int height);
	~LevelBackgroundPark();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

protected:

private:

};

#endif
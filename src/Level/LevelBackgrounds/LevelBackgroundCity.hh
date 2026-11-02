#ifndef LEVELLBACKGROUNDCITY_HH
#define LEVELLBACKGROUNDCITY_HH

#include "LevelBackground.hh"

class LevelBackgroundCity : public LevelBackground {
public:
	LevelBackgroundCity(const int width, const int height);
	~LevelBackgroundCity();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

protected:

private:

};

#endif
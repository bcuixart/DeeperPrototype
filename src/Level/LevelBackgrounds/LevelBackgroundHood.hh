#ifndef LEVELLBACKGROUNDHOOD_HH
#define LEVELLBACKGROUNDHOOD_HH

#include "LevelBackground.hh"

class LevelBackgroundHood : public LevelBackground {
public:
	LevelBackgroundHood(const int width, const int height);
	~LevelBackgroundHood();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

protected:

private:

};

#endif
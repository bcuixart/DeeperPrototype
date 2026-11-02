#ifndef LEVELLBACKGROUNDBEACH_HH
#define LEVELLBACKGROUNDBEACH_HH

#include "LevelBackground.hh"

class LevelBackgroundBeach : public LevelBackground {
public:
	LevelBackgroundBeach(const int width, const int height);
	~LevelBackgroundBeach();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

protected:

private:

};

#endif
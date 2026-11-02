#ifndef LEVELLBACKGROUNDSNOW_HH
#define LEVELLBACKGROUNDSNOW_HH

#include "LevelBackground.hh"

class LevelBackgroundSnow : public LevelBackground {
public:
	LevelBackgroundSnow(const int width, const int height);
	~LevelBackgroundSnow();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;

protected:

private:

};

#endif
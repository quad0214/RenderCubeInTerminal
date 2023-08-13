#pragma once

#define TERMINAL_FONT_ASPECT (8.0f / 18.0f)


// Constants for rendering
class Constants
{
public:
	static constexpr float PI = 3.14159265358979323846f;   // pi
	static constexpr int CUBE_LEN = 70; // length of cube side
	static constexpr int RENDER_SCREEN_WIDTH = 50; // width of screen where cube is rendered
	static constexpr int RENDER_SCREEN_HEIGHT = RENDER_SCREEN_WIDTH * TERMINAL_FONT_ASPECT; // height of screen where cube is rendered
	static constexpr float FOVY = 90.0f * PI / 180.0f;
	static constexpr float DST_NEAR_TO_FAR = 200;
	static constexpr float CAMERA_Z = -CUBE_LEN / 2.0f - 50.0f;
	static constexpr float ROTATION_X_SPEED = 10.0f * PI / 180.0f;
	static constexpr float ROTATION_Y_SPEED = 5.0f * PI / 180.0f;
	static constexpr float ROTATION_Z_SPEED = 2.5f * PI / 180.0f;
	static constexpr wchar_t RENDER_CLEAR_CHAR = L'.';

	static constexpr int CONSOLE_TEXT_HEIGHT = 5; // height of console infomration text. regarding only in windows
	static constexpr int CONSOLE_SCREEN_WIDTH = RENDER_SCREEN_WIDTH; //  console width. regarding only in windows
	static constexpr int CONSOLE_SCREEN_HEIGHT = RENDER_SCREEN_HEIGHT + CONSOLE_TEXT_HEIGHT; // console height. regarding only in windows
	static constexpr int CONSOLE_MAX_TEXT_LEN = CONSOLE_TEXT_HEIGHT * CONSOLE_SCREEN_WIDTH;
	static constexpr int TEXT_BUFFER_SIZE = CONSOLE_MAX_TEXT_LEN + 1;
	static constexpr wchar_t CONSOLE_CLEAR_CHAR = L' ';
	static constexpr int CONSOLE_TEXT_SECTION_START = 0;
	static constexpr int CONSOLE_RENDER_SECTION_START = CONSOLE_TEXT_SECTION_START + CONSOLE_MAX_TEXT_LEN;
};
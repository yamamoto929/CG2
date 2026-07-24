#pragma once
#include <cstdint>


enum class Key : uint8_t {
	NONE = 0x00,

	// 基本キー
	ESCAPE = 0x01,

	// キーボード上段の数字
	Digit1 = 0x02,
	Digit2 = 0x03,
	Digit3 = 0x04,
	Digit4 = 0x05,
	Digit5 = 0x06,
	Digit6 = 0x07,
	Digit7 = 0x08,
	Digit8 = 0x09,
	Digit9 = 0x0A,
	Digit0 = 0x0B,

	Minus = 0x0C,
	Equals = 0x0D,
	Backspace = 0x0E,
	Tab = 0x0F,

	// アルファベット上段
	Q = 0x10,
	W = 0x11,
	E = 0x12,
	R = 0x13,
	T = 0x14,
	Y = 0x15,
	U = 0x16,
	I = 0x17,
	O = 0x18,
	P = 0x19,

	LeftBracket = 0x1A,
	RightBracket = 0x1B,
	Enter = 0x1C,
	LeftControl = 0x1D,

	// アルファベット中段
	A = 0x1E,
	S = 0x1F,
	D = 0x20,
	F = 0x21,
	G = 0x22,
	H = 0x23,
	J = 0x24,
	K = 0x25,
	L = 0x26,

	Semicolon = 0x27,
	Apostrophe = 0x28,
	Grave = 0x29,
	LeftShift = 0x2A,
	Backslash = 0x2B,

	// アルファベット下段
	Z = 0x2C,
	X = 0x2D,
	C = 0x2E,
	V = 0x2F,
	B = 0x30,
	N = 0x31,
	M = 0x32,

	Comma = 0x33,
	Period = 0x34,
	Slash = 0x35,
	RightShift = 0x36,

	// テンキー・修飾キー
	NumpadMultiply = 0x37,
	LeftAlt = 0x38,
	Space = 0x39,
	CapsLock = 0x3A,

	// ファンクションキー
	F1 = 0x3B,
	F2 = 0x3C,
	F3 = 0x3D,
	F4 = 0x3E,
	F5 = 0x3F,
	F6 = 0x40,
	F7 = 0x41,
	F8 = 0x42,
	F9 = 0x43,
	F10 = 0x44,

	NumLock = 0x45,
	ScrollLock = 0x46,

	// テンキー
	Numpad7 = 0x47,
	Numpad8 = 0x48,
	Numpad9 = 0x49,
	NumpadSubtract = 0x4A,

	Numpad4 = 0x4B,
	Numpad5 = 0x4C,
	Numpad6 = 0x4D,
	NumpadAdd = 0x4E,

	Numpad1 = 0x4F,
	Numpad2 = 0x50,
	Numpad3 = 0x51,
	Numpad0 = 0x52,
	NumpadDecimal = 0x53,

	// 非USキーボード
	Oem102 = 0x56,

	F11 = 0x57,
	F12 = 0x58,
	F13 = 0x64,
	F14 = 0x65,
	F15 = 0x66,

	// 日本語・各国キーボード
	Kana = 0x70,
	AbntC1 = 0x73,
	Convert = 0x79,
	NoConvert = 0x7B,
	Yen = 0x7D,
	AbntC2 = 0x7E,

	// NEC PC-98・日本語キーボード
	NumpadEquals = 0x8D,

	PreviousTrack = 0x90,
	At = 0x91,
	Colon = 0x92,
	Underline = 0x93,
	Kanji = 0x94,
	Stop = 0x95,
	Ax = 0x96,
	Unlabeled = 0x97,

	NextTrack = 0x99,
	NumpadEnter = 0x9C,
	RightControl = 0x9D,

	// メディアキー
	Mute = 0xA0,
	Calculator = 0xA1,
	PlayPause = 0xA2,
	MediaStop = 0xA4,

	VolumeDown = 0xAE,
	VolumeUp = 0xB0,
	WebHome = 0xB2,

	NumpadComma = 0xB3,
	NumpadDivide = 0xB5,

	PrintScreen = 0xB7,
	RightAlt = 0xB8,

	Pause = 0xC5,

	// 移動キー
	Home = 0xC7,
	Up = 0xC8,
	PageUp = 0xC9,
	Left = 0xCB,
	Right = 0xCD,
	End = 0xCF,
	Down = 0xD0,
	PageDown = 0xD1,
	Insert = 0xD2,
	Delete = 0xD3,

	// Windows・システムキー
	LeftWindows = 0xDB,
	RightWindows = 0xDC,
	Application = 0xDD,
	Power = 0xDE,
	Sleep = 0xDF,
	Wake = 0xE3,

	// Web・アプリケーションキー
	WebSearch = 0xE5,
	WebFavorites = 0xE6,
	WebRefresh = 0xE7,
	WebStop = 0xE8,
	WebForward = 0xE9,
	WebBack = 0xEA,
	MyComputer = 0xEB,
	Mail = 0xEC,
	MediaSelect = 0xED,

	// DirectInput上で同じ値を持つ別名
	Return = Enter,
	Back = Backspace,

	LeftMenu = LeftAlt,
	RightMenu = RightAlt,

	Capital = CapsLock,

	NumpadStar = NumpadMultiply,
	NumpadMinus = NumpadSubtract,
	NumpadPlus = NumpadAdd,
	NumpadPeriod = NumpadDecimal,
	NumpadSlash = NumpadDivide,

	UpArrow = Up,
	DownArrow = Down,
	LeftArrow = Left,
	RightArrow = Right,

	Prior = PageUp,
	Next = PageDown,

	Apps = Application,
	SystemRequest = PrintScreen,

	// JISキーボードではPreviousTrackと同じコード
	Circumflex = PreviousTrack
};


enum class MouseButton {
	Left,
	Right,
	Middle
};

enum class GamepadButton {
	A,
	B,
	X,
	Y,
	DPadUp,
	DPadDown,
	DPadLeft,
	DPadRight,
	LeftShoulder,
	RightShoulder,
	LeftStick,
	RightStick,
	Start,
	Back
};


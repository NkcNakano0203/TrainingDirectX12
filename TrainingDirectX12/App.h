#pragma once
// インクルード
#include <Windows.h>
#include <cstdint>

class App
{
public:
	// パブリック変数

	// パブリック関数
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	// プライベート変数
	HINSTANCE m_hInst;
	HWND m_hWnd;
	uint32_t m_Width;
	uint32_t m_Height;

	// プライベート関数
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
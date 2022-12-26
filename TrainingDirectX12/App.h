#pragma once
// �C���N���[�h
#include <Windows.h>
#include <cstdint>

class App
{
public:
	// �p�u���b�N�ϐ�

	// �p�u���b�N�֐�
	App(uint32_t width, uint32_t height);
	~App();
	void Run();

private:
	// �v���C�x�[�g�ϐ�
	HINSTANCE m_hInst;
	HWND m_hWnd;
	uint32_t m_Width;
	uint32_t m_Height;

	// �v���C�x�[�g�֐�
	bool InitApp();
	void TermApp();
	bool InitWnd();
	void TermWnd();
	void MainLoop();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};
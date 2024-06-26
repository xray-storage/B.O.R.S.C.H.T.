#ifndef IM_CanvasH
#define IM_CanvasH

#include "IM_Window.h"

class ECORE_API IM_Canvas : public IM_Window
{
	private:
	int m_width, m_height;
	CRT* m_texture;
    HDC m_hdc;
    HBITMAP m_hbitmap, m_holdbm;

	public:
    IM_Canvas(int width, int height);
    ~IM_Canvas();

    void DestroyTexture();

    private:
    void CreateTexture();


    public:

    int Width() { return m_width; }
    int Height() { return m_height; }

    void Clear();

    HDC BeginPaint();
    void EndPaint();

    virtual void Render();
};

#endif

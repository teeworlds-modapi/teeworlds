#ifndef TU_CLIENT_GUI_TEXTEDIT_H
#define TU_CLIENT_GUI_TEXTEDIT_H

#include "label.h"

namespace tu
{
	
namespace gui
{

class CAbstractTextEdit : public CAbstractLabel
{
protected:
	bool m_Focus;
	bool m_Changes;
	bool m_MouseOver;
	CTextRenderer::CTextCache m_ComposingTextCache;
	bool m_Composing;
	CTextRenderer::CTextCursor m_TextCursor;
	CAssetPath m_ButtonStylePath;

protected:
	virtual void SaveFromTextBuffer() = 0;
	virtual void CopyToTextBuffer() = 0;

	void RefreshLabelStyle();

public:
	CAbstractTextEdit(class CContext *pConfig);
	
	virtual void Update();
	virtual void Render();
	
	virtual void OnMouseOver(int X, int Y, int RelX, int RelY, int KeyState);
	virtual void OnButtonClick(int X, int Y, int Button, int Count);
	virtual void OnInputEvent();
	
	void RemoveChanges();
	
	void OnFocusIn();
	void OnFocusOut();
	
	inline CAssetPath GetButtonStyle() const { return m_ButtonStylePath; }
	void SetButtonStyle(CAssetPath Path);
};

class CExternalTextEdit : public CAbstractTextEdit
{
protected:
	char* m_pText;
	int m_TextSize;
	
	virtual void SaveFromTextBuffer();
	virtual void CopyToTextBuffer();

public:
	CExternalTextEdit(class CContext *pConfig, char* pText, int TextSize);
};

}

}

#endif

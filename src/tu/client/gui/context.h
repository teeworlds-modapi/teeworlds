#ifndef TU_CLIENT_GUI_CONTEXT_H
#define TU_CLIENT_GUI_CONTEXT_H

#include <tu/client/kernel.h>
#include <tu/client/assetpath.h>
#include <tu/client/gui/rect.h>
#include <game/client/localization.h>
#include <tu/client/localization.h>

namespace tu
{
	
namespace gui
{

class CContext : public CKernel::CGuest, CLocalization::IListener
{
public:
	class CGuest
	{
	private:
		CContext* m_pContext;
		
	public:
		CGuest(CContext* pContext) : m_pContext(pContext) { }
		CKernel* Kernel() { return m_pContext->Kernel(); }
		CContext* Context() { return m_pContext; }
		#define TU_MACRO_KERNELCOMPONENT(TypeName, FuncName, MemberName, IsAllocated) inline class TypeName* FuncName() { return m_pContext->FuncName(); };
		#include <tu/client/kernelmacro.h>
		#undef TU_MACRO_KERNELCOMPONENT
	};

public:
	enum
	{
		ALIGNMENT_LEFT = 0,
		ALIGNMENT_CENTER,
		ALIGNMENT_RIGHT,
	};
	
protected:
	CRect m_DrawRect;
	float m_ImageScale;
	float m_GuiScale;
	bool m_LocalizationUpdated;
	
	array<class CPopup*> m_pPopups;
	class CWidget* m_pMainWidget;
	
	//Input
	ivec2 m_MouseDelta;
	ivec2 m_MousePos;
	static int s_aButtons[];
	
	int m_Button[5];
	
	CAssetPath m_LabelStyle;
	CAssetPath m_LabelHeaderStyle;
	CAssetPath m_ButtonStyle;
	CAssetPath m_ToggleStyle;
	CAssetPath m_TextEntryStyle;
	CAssetPath m_NumericEntryStyle;
	CAssetPath m_SliderStyle;
	CAssetPath m_ScrollbarStyle;
	CAssetPath m_TabsStyle;
	CAssetPath m_PopupStyle;
	CAssetPath m_CursorPath;
	bool m_ShowCursor;
	
public:
	CContext(CKernel* pKernel);
	virtual ~CContext();
	
	void Init(const CRect& DrawRect);
	
	virtual void OnLocalizationModified();
	
	virtual void CreateMainWidget() = 0;
	virtual void DoShortcuts() = 0;
	
	void Update();
	void Render();
	virtual void RenderBackground() = 0;
	
	void DisplayPopup(CPopup* pPopup);
	
	const ivec2& GetMousePos() { return m_MousePos; }
	
	inline CAssetPath GetLabelStyle() const { return m_LabelStyle; }
	inline CAssetPath GetLabelHeaderStyle() const { return m_LabelHeaderStyle; }
	inline CAssetPath GetButtonStyle() const { return m_ButtonStyle; }
	inline CAssetPath GetToggleStyle() const { return m_ToggleStyle; }
	inline CAssetPath GetTextEntryStyle() const { return m_TextEntryStyle; }
	inline CAssetPath GetNumericEntryStyle() const { return m_NumericEntryStyle; }
	inline CAssetPath GetSliderStyle() const { return m_SliderStyle; }
	inline CAssetPath GetScrollbarStyle() const { return m_ScrollbarStyle; }
	inline CAssetPath GetTabsStyle() const { return m_TabsStyle; }
	inline CAssetPath GetPopupStyle() const { return m_PopupStyle; }
	
	inline float GetImageScale() const { return m_ImageScale; }
	inline float GetGuiScale() const { return m_GuiScale; }
	inline int ApplyGuiScale(int Value) const { return static_cast<int>(Value*m_GuiScale); }
	
	inline void SetCursorSprite(CAssetPath CursorPath) { m_CursorPath = CursorPath; }
	
	inline bool LocalizationUpdated() const { return m_LocalizationUpdated; }
};

}

}

#endif

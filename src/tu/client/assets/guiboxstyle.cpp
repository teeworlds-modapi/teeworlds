#include "guiboxstyle.h"

#include <engine/shared/datafile.h>
#include <tu/client/assetsrenderer.h>

namespace tu
{

CAsset_GuiBoxStyle::CAsset_GuiBoxStyle() :
	m_Margin(0),
	m_Padding(0),
	m_Spacing(0),
	m_MinWidth(-1),
	m_MinHeight(-1)
{
	
}

/* IO *****************************************************************/

void CAsset_GuiBoxStyle::InitFromAssetsFile(CDataFileReader* pFileReader, const CStorageType* pItem)
{
	// copy name
	SetName((char *)pFileReader->GetData(pItem->m_Name));
		
	m_RectPath = pItem->m_RectPath;
	m_MinWidth = pItem->m_MinWidth;
	m_MinHeight = pItem->m_MinHeight;
	m_Margin = pItem->m_Margin;
	m_Padding = pItem->m_Padding;
	m_Spacing = pItem->m_Spacing;
}

void CAsset_GuiBoxStyle::SaveInAssetsFile(CDataFileWriter* pFileWriter, int Position)
{
	CStorageType Item;
	Item.m_Name = pFileWriter->AddData(str_length(m_aName)+1, m_aName);
	
	Item.m_RectPath = m_RectPath;
	Item.m_MinWidth = m_MinWidth;
	Item.m_MinHeight = m_MinHeight;
	Item.m_Margin = m_Margin;
	Item.m_Padding = m_Padding;
	Item.m_Spacing = m_Spacing;
	
	pFileWriter->AddItem(CAssetPath::TypeToStoredType(TypeId), Position, sizeof(CStorageType), &Item);
}

/* VALUE INT **********************************************************/

template<>
int CAsset_GuiBoxStyle::GetValue(int ValueType, int PathInt, int DefaultValue) const
{
	switch(ValueType)
	{
		TU_ASSET_GET_FUNC_IMPL_FUNC(int, MINWIDTH, GetMinWidth)
		TU_ASSET_GET_FUNC_IMPL_FUNC(int, MINHEIGHT, GetMinHeight)
		TU_ASSET_GET_FUNC_IMPL_FUNC(int, MARGIN, GetMargin)
		TU_ASSET_GET_FUNC_IMPL_FUNC(int, PADDING, GetPadding)
		TU_ASSET_GET_FUNC_IMPL_FUNC(int, SPACING, GetSpacing)
	}
	
	TU_ASSET_GET_FUNC_IMPL_DEFAULT(int)
}
	
template<>
bool CAsset_GuiBoxStyle::SetValue(int ValueType, int PathInt, int Value)
{
	switch(ValueType)
	{
		TU_ASSET_SET_FUNC_IMPL_FUNC(int, MINWIDTH, SetMinWidth)
		TU_ASSET_SET_FUNC_IMPL_FUNC(int, MINHEIGHT, SetMinHeight)
		TU_ASSET_SET_FUNC_IMPL_FUNC(int, MARGIN, SetMargin)
		TU_ASSET_SET_FUNC_IMPL_FUNC(int, PADDING, SetPadding)
		TU_ASSET_SET_FUNC_IMPL_FUNC(int, SPACING, SetSpacing)
	}
	
	TU_ASSET_SET_FUNC_IMPL_DEFAULT(int)
}

/* VALUE ASSETPATH **********************************************************/

template<>
CAssetPath CAsset_GuiBoxStyle::GetValue(int ValueType, int PathInt, CAssetPath DefaultValue) const
{
	switch(ValueType)
	{
		TU_ASSET_GET_FUNC_IMPL_FUNC(CAssetPath, RECTSTYLEPATH, GetRectPath)
	}
	
	TU_ASSET_GET_FUNC_IMPL_DEFAULT(CAssetPath)
}
	
template<>
bool CAsset_GuiBoxStyle::SetValue(int ValueType, int PathInt, CAssetPath Value)
{
	switch(ValueType)
	{
		TU_ASSET_SET_FUNC_IMPL_FUNC(CAssetPath, RECTSTYLEPATH, SetRectPath)
	}
	
	TU_ASSET_SET_FUNC_IMPL_DEFAULT(CAssetPath)
}

}

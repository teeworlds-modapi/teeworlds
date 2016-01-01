#include <modapi/server/modcreator.h>
#include <modapi/graphics.h>

#include <engine/storage.h>
#include <engine/shared/datafile.h>
#include <engine/graphics.h>

#include <engine/external/pnglite/pnglite.h>

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetOuter(int Width, const vec4& Color)
{
	m_OuterWidth = Width;
	m_OuterColor = ModAPI_ColorToInt(Color);
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetInner(int Width, const vec4& Color)
{
	m_InnerWidth = Width;
	m_InnerColor = ModAPI_ColorToInt(Color);
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetLineRepeatedSprite(int SpriteId1, int SpriteSizeX, int SpriteSizeY)
{
	m_LineSpriteType = MODAPI_LINESTYLE_SPRITETYPE_REPEATED;
	m_LineSprite1 = SpriteId1;
	m_LineSprite2 = -1;
	m_LineSpriteSizeX = SpriteSizeX;
	m_LineSpriteSizeY = SpriteSizeY;
	m_LineSpriteOverlapping = 0;
	m_LineSpriteAnimationSpeed = -1;
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetStartPointSprite(int SpriteId1, int X, int Y, int W, int H)
{
	m_StartPointSprite1 = SpriteId1;
	m_StartPointSprite2 = -1;
	m_StartPointSpriteX = X;
	m_StartPointSpriteY = Y;
	m_StartPointSpriteSizeX = W;
	m_StartPointSpriteSizeY = H;
	m_StartPointSpriteAnimationSpeed = -1;
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetStartPointAnimatedSprite(int SpriteId1, int SpriteId2, int X, int Y, int W, int H, int Speed)
{
	m_StartPointSprite1 = SpriteId1;
	m_StartPointSprite2 = SpriteId2;
	m_StartPointSpriteX = X;
	m_StartPointSpriteY = Y;
	m_StartPointSpriteSizeX = W;
	m_StartPointSpriteSizeY = H;
	m_StartPointSpriteAnimationSpeed = Speed;
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetEndPointSprite(int SpriteId1, int X, int Y, int W, int H)
{
	m_EndPointSprite1 = SpriteId1;
	m_EndPointSprite2 = -1;
	m_EndPointSpriteX = X;
	m_EndPointSpriteY = Y;
	m_EndPointSpriteSizeX = W;
	m_EndPointSpriteSizeY = H;
	m_EndPointSpriteAnimationSpeed = -1;
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetEndPointAnimatedSprite(int SpriteId1, int SpriteId2, int X, int Y, int W, int H, int Speed)
{
	m_EndPointSprite1 = SpriteId1;
	m_EndPointSprite2 = SpriteId2;
	m_EndPointSpriteX = X;
	m_EndPointSpriteY = Y;
	m_EndPointSpriteSizeX = W;
	m_EndPointSpriteSizeY = H;
	m_EndPointSpriteAnimationSpeed = Speed;
	
	return *this;
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::CModAPI_LineStyleCreator::SetLineAnimation(int Type, int Speed)
{
	m_AnimationType = Type;
	m_AnimationSpeed = Speed;
	
	return *this;
}

CModAPI_ModCreator::CModAPI_ModCreator()
{
	
}

int CModAPI_ModCreator::AddImage(IStorage* pStorage, const char* pFilename)
{
	CModAPI_ModItem_Image Image;
	
	char aCompleteFilename[512];
	unsigned char *pBuffer;
	png_t Png; // ignore_convention

	// open file for reading
	png_init(0,0); // ignore_convention

	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL, aCompleteFilename, sizeof(aCompleteFilename));
	if(File)
	{
		io_close(File);
	}
	else
	{
		dbg_msg("mod", "failed to open file. filename='%s'", pFilename);
		return -1;
	}

	int Error = png_open_file(&Png, aCompleteFilename); // ignore_convention
	if(Error != PNG_NO_ERROR)
	{
		dbg_msg("mod", "failed to open file. filename='%s'", aCompleteFilename);
		if(Error != PNG_FILE_ERROR)
		{
			png_close_file(&Png); // ignore_convention
		}
		return -1;
	}

	if(Png.depth != 8 || (Png.color_type != PNG_TRUECOLOR && Png.color_type != PNG_TRUECOLOR_ALPHA) || Png.width > (2<<12) || Png.height > (2<<12)) // ignore_convention
	{
		dbg_msg("mod", "invalid format. filename='%s'", aCompleteFilename);
		png_close_file(&Png); // ignore_convention
		return -1;
	}

	pBuffer = (unsigned char *)mem_alloc(Png.width * Png.height * Png.bpp, 1); // ignore_convention
	png_get_data(&Png, pBuffer); // ignore_convention
	png_close_file(&Png); // ignore_convention

	Image.m_Id = m_Images.size();
	Image.m_Width = Png.width; // ignore_convention
	Image.m_Height = Png.height; // ignore_convention
	if(Png.color_type == PNG_TRUECOLOR) // ignore_convention
		Image.m_Format = CImageInfo::FORMAT_RGB;
	else if(Png.color_type == PNG_TRUECOLOR_ALPHA) // ignore_convention
		Image.m_Format = CImageInfo::FORMAT_RGBA;
	
	m_Images.add(Image);
	m_ImagesData.add(pBuffer);
	
	return Image.m_Id;
}

int CModAPI_ModCreator::AddSprite(int ImageId, int External, int x, int y, int w, int h, int gx, int gy)
{	
	CModAPI_ModItem_Sprite sprite;
	sprite.m_Id = m_Sprites.size();
	sprite.m_X = x;
	sprite.m_Y = y;
	sprite.m_W = w;
	sprite.m_H = h;
	sprite.m_External = External;
	sprite.m_ImageId = ImageId;
	sprite.m_GridX = gx;
	sprite.m_GridY = gy;
	
	m_Sprites.add(sprite);
	
	return sprite.m_Id;
}

int CModAPI_ModCreator::AddSpriteInternal(int ImageId, int x, int y, int w, int h, int gx, int gy)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't add the internal sprite : wrong image id (%i)", ImageId);
		return -1;
	}
	return AddSprite(ImageId, 0, x, y, w, h, gx, gy);
}

int CModAPI_ModCreator::AddSpriteExternal(int ImageId, int x, int y, int w, int h, int gx, int gy)
{	
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't add the external sprite : wrong image id (%i)", ImageId);
		return -1;
	}
	return AddSprite(ImageId, 1, x, y, w, h, gx, gy);
}

CModAPI_ModCreator::CModAPI_LineStyleCreator& CModAPI_ModCreator::AddLineStyle()
{
	int Id = m_LineStyles.size();
	
	m_LineStyles.set_size(m_LineStyles.size()+1);
	
	CModAPI_ModCreator::CModAPI_LineStyleCreator& LineStyle = m_LineStyles[Id];
	LineStyle.m_Id = Id;
	LineStyle.m_OuterWidth = -1;
	LineStyle.m_OuterColor = 0;
	LineStyle.m_InnerWidth = -1;
	LineStyle.m_InnerColor = 0;

	LineStyle.m_LineSpriteType = MODAPI_LINESTYLE_SPRITETYPE_REPEATED;
	LineStyle.m_LineSprite1 = -1;
	LineStyle.m_LineSprite2 = -1;
	LineStyle.m_LineSpriteSizeX = 0;
	LineStyle.m_LineSpriteSizeY = 0;
	LineStyle.m_LineSpriteOverlapping = 0;
	LineStyle.m_LineSpriteAnimationSpeed = -1;

	//Start point sprite
	LineStyle.m_StartPointSprite1 = -1;
	LineStyle.m_StartPointSprite2 = -1;
	LineStyle.m_StartPointSpriteX = 0;
	LineStyle.m_StartPointSpriteY = 0;
	LineStyle.m_StartPointSpriteSizeX = 0;
	LineStyle.m_StartPointSpriteSizeY = 0;
	LineStyle.m_StartPointSpriteAnimationSpeed = -1;
	
	//End point prite
	LineStyle.m_EndPointSprite1 = -1;
	LineStyle.m_EndPointSprite2 = -1;
	LineStyle.m_EndPointSpriteX = 0;
	LineStyle.m_EndPointSpriteY = 0;
	LineStyle.m_EndPointSpriteSizeX = 0;
	LineStyle.m_EndPointSpriteSizeY = 0;
	LineStyle.m_EndPointSpriteAnimationSpeed = -1;
	
	//General information
	LineStyle.m_AnimationType = MODAPI_LINESTYLE_ANIMATION_NONE;
	LineStyle.m_AnimationSpeed = -1;
	
	return LineStyle;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::AddSkinModifier()
{
	int Id = m_SkinModifiers.size();
	m_SkinModifiers.set_size(m_SkinModifiers.size()+1);
	CModAPI_ModCreator::CModAPI_SkinModifierCreator &SkinModifier = m_SkinModifiers[Id];

	SkinModifier.m_Id = Id;

	return SkinModifier.SetVanillaSkin(0); // initialize everything to change nothing
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetVanillaSkin(const char *pName)
{
	// body
	m_BodyFlags = MODAPI_SKINMODIFIER_PART_KEEP;
	m_BodyColor = -1;
	m_BodyImage = -1;

	// marking
	m_MarkingFlags = MODAPI_SKINMODIFIER_PART_KEEP;
	m_MarkingColor = -1;
	m_MarkingImage = -1;

	// decoration
	m_DecoFlags = MODAPI_SKINMODIFIER_PART_KEEP;
	m_DecoColor = -1;
	m_DecoImage = -1;

	// hands
	m_HandsFlags = MODAPI_SKINMODIFIER_PART_KEEP;
	m_HandsColor = -1;
	m_HandsImage = -1;

	// feet
	m_FeetFlags = MODAPI_SKINMODIFIER_PART_KEEP;
	m_FeetColor = -1;
	m_FeetImage = -1;

	// eyes
	m_EyesFlags = MODAPI_SKINMODIFIER_PART_KEEP;
	m_EyesColor = -1;
	m_EyesImage = -1;

	m_HookStyle = -1; // a line style ID


	// TODO: parse the json file (pName) here!!
	return *this;
}

// skin modifier - body
CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetBodyColor(const vec4& Color)
{
	m_BodyColor = ModAPI_ColorToInt(Color);
	m_BodyFlags |= MODAPI_SKINMODIFIER_PART_CUSTOMCOLOR;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetBodyInternalSkin(int ImageId)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't set internal skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_BodyImage = ImageId;
	m_BodyFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_BodyFlags &= ~MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetBodyExternalSkin(int ImageId)
{
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't set the external skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_BodyImage = ImageId;
	m_BodyFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_BodyFlags |= MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

// skin modifier - marking
CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetMarkingColor(const vec4& Color)
{
	m_MarkingColor = ModAPI_ColorToInt(Color);
	m_MarkingFlags |= MODAPI_SKINMODIFIER_PART_CUSTOMCOLOR;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetMarkingInternalSkin(int ImageId)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't set internal skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_MarkingImage = ImageId;
	m_MarkingFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_MarkingFlags &= ~MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetMarkingExternalSkin(int ImageId)
{
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't set the external skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_MarkingImage = ImageId;
	m_MarkingFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_MarkingFlags |= MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

// skin modifier - decoration
CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetDecorationColor(const vec4& Color)
{
	m_DecoColor = ModAPI_ColorToInt(Color);
	m_DecoFlags |= MODAPI_SKINMODIFIER_PART_CUSTOMCOLOR;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetDecorationInternalSkin(int ImageId)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't set internal skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_DecoImage = ImageId;
	m_DecoFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_DecoFlags &= ~MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetDecorationExternalSkin(int ImageId)
{
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't set the external skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_DecoImage = ImageId;
	m_DecoFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_DecoFlags |= MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

// skin modifier - hands
CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetHandsColor(const vec4& Color)
{
	m_HandsColor = ModAPI_ColorToInt(Color);
	m_HandsFlags |= MODAPI_SKINMODIFIER_PART_CUSTOMCOLOR;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetHandsInternalSkin(int ImageId)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't set internal skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_HandsImage = ImageId;
	m_HandsFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_HandsFlags &= ~MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetHandsExternalSkin(int ImageId)
{
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't set the external skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_HandsImage = ImageId;
	m_HandsFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_HandsFlags |= MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

// skin modifier - feet
CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetFeetColor(const vec4& Color)
{
	m_FeetColor = ModAPI_ColorToInt(Color);
	m_FeetFlags |= MODAPI_SKINMODIFIER_PART_CUSTOMCOLOR;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetFeetInternalSkin(int ImageId)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't set internal skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_FeetImage = ImageId;
	m_FeetFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_FeetFlags &= ~MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetFeetExternalSkin(int ImageId)
{
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't set the external skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_FeetImage = ImageId;
	m_FeetFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_FeetFlags |= MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

// skin modifier - eyes
CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetEyesColor(const vec4& Color)
{
	m_EyesColor = ModAPI_ColorToInt(Color);
	m_EyesFlags |= MODAPI_SKINMODIFIER_PART_CUSTOMCOLOR;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetEyesInternalSkin(int ImageId)
{
	if(ImageId >= MODAPI_NB_INTERNALIMG || ImageId < 0)
	{
		dbg_msg("mod", "can't set internal skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_EyesImage = ImageId;
	m_EyesFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_EyesFlags &= ~MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}

CModAPI_ModCreator::CModAPI_SkinModifierCreator& CModAPI_ModCreator::CModAPI_SkinModifierCreator::SetEyesExternalSkin(int ImageId)
{
	if(ImageId >= m_Images.size() || ImageId < 0)
	{
		dbg_msg("mod", "can't set the external skin : wrong image id (%i)", ImageId);
		return *this;
	}
	m_EyesImage = ImageId;
	m_EyesFlags |= MODAPI_SKINMODIFIER_PART_CHANGEFILE;
	m_EyesFlags |= MODAPI_SKINMODIFIER_PART_EXTERNALFILE;
	return *this;
}



int CModAPI_ModCreator::Save(class IStorage *pStorage, const char *pFileName)
{
	CDataFileWriter df;
	if(!df.Open(pStorage, pFileName))
	{
		dbg_msg("mod", "can't create the mod file %s", pFileName);
		return 0;
	}
	
	//Save images
	for(int i=0; i<m_Images.size(); i++)
	{
		CModAPI_ModItem_Image* pImage = &m_Images[i];
		int PixelSize = pImage->m_Format == CImageInfo::FORMAT_RGB ? 3 : 4;
		pImage->m_ImageData = df.AddData(pImage->m_Width*pImage->m_Height*PixelSize, m_ImagesData[i]);
		df.AddItem(MODAPI_MODITEMTYPE_IMAGE, i, sizeof(CModAPI_ModItem_Image), pImage);
	}
	
	//Save sprites
	for(int i=0; i<m_Sprites.size(); i++)
	{
		df.AddItem(MODAPI_MODITEMTYPE_SPRITE, i, sizeof(CModAPI_ModItem_Sprite), &m_Sprites[i]);
	}
	
	//Save line styles
	for(int i=0; i<m_LineStyles.size(); i++)
	{
		df.AddItem(MODAPI_MODITEMTYPE_LINESTYLE, i, sizeof(CModAPI_ModItem_LineStyle), &m_LineStyles[i]);
	}
	
	//Save skin changers
	for(int i=0; i<m_SkinModifiers.size(); i++)
	{
		df.AddItem(MODAPI_MODITEMTYPE_SKINMODIFIER, i, sizeof(CModAPI_ModItem_SkinModifier), &m_SkinModifiers[i]);
		//df.
	}

	df.Finish();
	
	return 1;
}

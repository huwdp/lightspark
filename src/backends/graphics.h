/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2010-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef BACKENDS_GRAPHICS_H
#define BACKENDS_GRAPHICS_H 1

#define CHUNKSIZE 128

#include "compat.h"
#include <vector>
#include "swftypes.h"
#include "threading.h"
#include <cairo.h>
#include <pango/pango.h>
#include "backends/geometry.h"
#include "memory_support.h"

namespace lightspark
{

class DisplayObject;
class InvalidateQueue;
class ColorTransform;

class TextureChunk
{
friend class GLRenderContext;
friend class CairoRenderContext;
friend class RenderThread;
private:
	/*
	 * For GLRenderContext texId is an OpenGL texture id and chunks is an array of used
	 * chunks inside such texture.
	 * For CairoRenderContext texId is an arbitrary id for the texture and chunks is
	 * not used.
	 */
	uint32_t* chunks;
	uint32_t texId;
	TextureChunk(uint32_t w, uint32_t h);
public:
	TextureChunk():chunks(NULL),texId(0),width(0),height(0){}
	TextureChunk(const TextureChunk& r);
	TextureChunk& operator=(const TextureChunk& r);
	~TextureChunk();
	bool resizeIfLargeEnough(uint32_t w, uint32_t h);
	uint32_t getNumberOfChunks() const { return ((width+CHUNKSIZE-1)/CHUNKSIZE)*((height+CHUNKSIZE-1)/CHUNKSIZE); }
	bool isValid() const { return chunks; }
	void makeEmpty();
	uint32_t width;
	uint32_t height;
};

class CachedSurface
{
public:
	CachedSurface():xOffset(0),yOffset(0),alpha(1.0){}
	TextureChunk tex;
	int32_t xOffset;
	int32_t yOffset;
	float alpha;
};

class ITextureUploadable
{
protected:
	~ITextureUploadable(){}
public:
	virtual void sizeNeeded(uint32_t& w, uint32_t& h) const=0;
	/*
		Upload data to memory mapped to the graphics card (note: size is guaranteed to be enough
	*/
	virtual void upload(uint8_t* data, uint32_t w, uint32_t h) const=0;
	virtual const TextureChunk& getTexture()=0;
	/*
		Signal the completion of the upload to the texture
		NOTE: fence may be called on shutdown even if the upload has not happen, so be ready for this event
	*/
	virtual void uploadFence()=0;
};

class IDrawable
{
public:
	enum MASK_MODE { HARD_MASK = 0, SOFT_MASK };
	struct MaskData
	{
		IDrawable* m;
		MASK_MODE maskMode;
		MaskData(IDrawable* _m, MASK_MODE _mm):m(_m),maskMode(_mm){}
	};
protected:
	/*
	 * The masks to be applied
	 */
	std::vector<MaskData> masks;
	int32_t width;
	int32_t height;
	/*
	   The minimal x coordinate for all the points being drawn, in local coordinates
	*/
	int32_t xOffset;
	/*
	   The minimal y coordinate for all the points being drawn, in local coordinates
	*/
	int32_t yOffset;
	float alpha;
public:
	IDrawable(int32_t w, int32_t h, int32_t x, int32_t y, float a, const std::vector<MaskData>& m):
		masks(m),width(w),height(h),xOffset(x),yOffset(y),alpha(a){}
	virtual ~IDrawable();
	/*
	 * This method returns a raster buffer of the image
	 * The various implementation are responsible for applying the
	 * masks
	 */
	virtual uint8_t* getPixelBuffer()=0;
	/*
	 * This method creates a cairo path that can be used as a mask for
	 * another object
	 */
	virtual void applyCairoMask(cairo_t* cr, int32_t offsetX, int32_t offsetY, float scalex, float scaley) const = 0;
	int32_t getWidth() const { return width; }
	int32_t getHeight() const { return height; }
	int32_t getXOffset() const { return xOffset; }
	int32_t getYOffset() const { return yOffset; }
	float getAlpha() const { return alpha; }
};

class AsyncDrawJob: public IThreadJob, public ITextureUploadable
{
private:
	IDrawable* drawable;
	/**
	 * The DisplayObject owning this render request. We incRef/decRef it
	 * in our constructor/destructor to make sure that it does not go away
	 */
	_R<DisplayObject> owner;
	uint8_t* surfaceBytes;
	bool uploadNeeded;
public:
	/*
	 * @param o The DisplayObject that is being rendered. It is a reference to
	 * make sure the object survives until the end of the rendering
	 * @param d IDrawable to be rendered asynchronously. The pointer is now
	 * owned by this instance
	 */
	AsyncDrawJob(IDrawable* d, _R<DisplayObject> o,int32_t flushstep);
	~AsyncDrawJob();
	//IThreadJob interface
	void execute();
	void threadAbort();
	void jobFence();
	//ITextureUploadable interface
	void upload(uint8_t* data, uint32_t w, uint32_t h) const;
	void sizeNeeded(uint32_t& w, uint32_t& h) const;
	const TextureChunk& getTexture();
	void uploadFence();
};

/**
	The base class for render jobs based on cairo
	Stores an internal copy of the data to be rendered
*/
class CairoRenderer: public IDrawable
{
protected:
	/*
	   The scale to be applied in both the x and y axis.
	   Useful to adapt points defined in pixels and twips (1/20 of pixel)
	*/
	const float scaleFactor;
	bool smoothing;
	/**
	  The whole transformation matrix that is applied to the rendered object
	*/
	MATRIX matrix;
	static void cairoClean(cairo_t* cr);
	cairo_surface_t* allocateSurface(uint8_t*& buf);
	virtual void executeDraw(cairo_t* cr, float scalex, float scaley)=0;
	static void copyRGB15To24(uint32_t& dest, uint8_t* src);
	static void copyRGB24To24(uint32_t& dest, uint8_t* src);
public:
	CairoRenderer(const MATRIX& _m, int32_t _x, int32_t _y, int32_t _w, int32_t _h, float _s, float _a, const std::vector<MaskData>& m,bool _smoothing);
	//IDrawable interface
	uint8_t* getPixelBuffer();
	/*
	 * Converts data (which is in RGB format) to the format internally used by cairo.
	 */
	static void convertBitmapToCairo(std::vector<uint8_t, reporter_allocator<uint8_t>>& data, uint8_t* inData, uint32_t width,
					 uint32_t height, size_t* dataSize, size_t* stride, uint32_t bpp);
	/*
	 * Converts data (which is in ARGB or RGBA(png) format) to the format internally used by cairo.
	 */
	static void convertBitmapWithAlphaToCairo(std::vector<uint8_t, reporter_allocator<uint8_t>>& data, uint8_t* inData, uint32_t width,
			uint32_t height, size_t* dataSize, size_t* stride, bool frompng);
};

class CairoTokenRenderer : public CairoRenderer
{
private:
	static cairo_pattern_t* FILLSTYLEToCairo(const FILLSTYLE& style, double scaleCorrection, ColorTransform *colortransform, float scalex, float scaley);
	static bool cairoPathFromTokens(cairo_t* cr, const tokensVector &tokens, double scaleCorrection, bool skipFill, lightspark::ColorTransform *colortransform, float scalex, float scaley);
	static void quadraticBezier(cairo_t* cr, double control_x, double control_y, double end_x, double end_y);
	/*
	   The tokens to be drawn
	*/
	const tokensVector tokens;
	_NR<ColorTransform> colortransform;
	/*
	 * This is run by CairoRenderer::execute()
	 */
	void executeDraw(cairo_t* cr, float scalex, float scaley);
	void applyCairoMask(cairo_t* cr, int32_t offsetX, int32_t offsetY, float scalex, float scaley) const;
public:
	/*
	   CairoTokenRenderer constructor

	   @param _o Owner of the surface _t. See comments on 'owner' member.
	   @param _t GL surface where the final drawing will be uploaded
	   @param _g The tokens to be drawn. This is copied internally.
	   @param _m The whole transformation matrix
	   @param _s The scale factor to be applied in both the x and y axis
	   @param _a The alpha factor to be applied
	   @param _ms The masks that must be applied
	   @param _smoothing indicates if the tokens should be rendered with antialiasing
	*/
	CairoTokenRenderer(const tokensVector& _g, const MATRIX& _m,
			int32_t _x, int32_t _y, int32_t _w, int32_t _h,
		    float _s, float _a, const std::vector<MaskData>& _ms,bool _smoothing,
			ColorTransform* _ct);
	/*
	   Hit testing helper. Uses cairo to find if a point in inside the shape

	   @param tokens The tokens of the shape being tested
	   @param scaleFactor The scale factor to be applied
	   @param x The X in local coordinates
	   @param y The Y in local coordinates
	*/
	static bool hitTest(const tokensVector& tokens, float scaleFactor, number_t x, number_t y);
};

class TextData
{
public:
	/* the default values are from the spec for flash.text.TextField and flash.text.TextFormat */
	TextData() : width(100), height(100),leading(0), textWidth(0), textHeight(0), font("Times New Roman"),fontID(UINT32_MAX), scrollH(0), scrollV(1), background(false), backgroundColor(0xFFFFFF),
		border(false), borderColor(0x000000), multiline(false), textColor(0x000000),
		autoSize(AS_NONE), fontSize(12), wordWrap(false),caretblinkstate(false) {}
	uint32_t width;
	uint32_t height;
	uint32_t leading;
	uint32_t textWidth;
	uint32_t textHeight;
	tiny_string text;
	tiny_string font;
	uint32_t fontID;
	int32_t scrollH; // pixels, 0-based
	int32_t scrollV; // lines, 1-based
	bool background;
	RGB backgroundColor;
	bool border;
	RGB borderColor;
	bool multiline;
	RGB textColor;
	enum AUTO_SIZE {AS_NONE = 0, AS_LEFT, AS_RIGHT, AS_CENTER };
	AUTO_SIZE autoSize;
	uint32_t fontSize;
	bool wordWrap;
	bool caretblinkstate;
};

class LineData {
public:
	LineData(int32_t x, int32_t y, int32_t _width,
		 int32_t _height, int32_t _firstCharOffset, int32_t _length,
		 number_t _ascent, number_t _descent, number_t _leading,
		 number_t _indent):
		extents(x, x+_width, y, y+_height), 
		firstCharOffset(_firstCharOffset), length(_length),
		ascent(_ascent), descent(_descent), leading(_leading),
		indent(_indent) {}
	// position and size
	RECT extents;
	// Offset of the first character on this line
	int32_t firstCharOffset;
	// length of the line in characters
	int32_t length;
	number_t ascent;
	number_t descent;
	number_t leading;
	number_t indent;
};

class CairoPangoRenderer : public CairoRenderer
{
	/*
	 * This is run by CairoRenderer::execute()
	 */
	void executeDraw(cairo_t* cr, float scalex, float scaley);
	TextData textData;
	uint32_t caretIndex;
	static void pangoLayoutFromData(PangoLayout* layout, const TextData& tData);
	void applyCairoMask(cairo_t* cr, int32_t offsetX, int32_t offsetY, float scalex, float scaley) const;
	static PangoRectangle lineExtents(PangoLayout *layout, int lineNumber);
public:
	CairoPangoRenderer(const TextData& _textData, const MATRIX& _m,
			int32_t _x, int32_t _y, int32_t _w, int32_t _h, float _s, float _a, const std::vector<MaskData>& _ms,bool _smoothing,uint32_t _ci)
		: CairoRenderer(_m,_x,_y,_w,_h,_s,_a,_ms,_smoothing), textData(_textData),caretIndex(_ci) {}
	/**
		Helper. Uses Pango to find the size of the textdata
		@param _texttData The textData being tested
		@param w,h,tw,th are the (text)width and (text)height of the textData.
	*/
	static bool getBounds(const TextData& _textData, uint32_t& w, uint32_t& h, uint32_t& tw, uint32_t& th);
	static std::vector<LineData> getLineData(const TextData& _textData);
};

class InvalidateQueue
{
public:
	virtual ~InvalidateQueue(){}
	//Invalidation queue management
	virtual void addToInvalidateQueue(_R<DisplayObject> d) = 0;
};

class SoftwareInvalidateQueue: public InvalidateQueue
{
public:
	std::list<_R<DisplayObject>> queue;
	void addToInvalidateQueue(_R<DisplayObject> d);
};

};
#endif /* BACKENDS_GRAPHICS_H */

//
// PFont.hh for pekwm
// Copyright (C) 2003-2020 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_PFONT_HH_
#define _PEKWM_PFONT_HH_

#include "config.h"

#include <string>

#include "pekwm.hh"

extern "C" {
#ifdef PEKWM_HAVE_XFT
#include <X11/Xft/Xft.h>
#endif // PEKWM_HAVE_XFT
}

class PFont
{
public:
	enum Type {
		FONT_TYPE_X11, FONT_TYPE_XFT, FONT_TYPE_XMB, FONT_TYPE_NO
	};
	enum TrimType {
		FONT_TRIM_END, FONT_TRIM_MIDDLE
	};

	class Color {
	public:
		Color(void);
		~Color(void);

		XColor *getFg(void) { return _fg; }
		XColor *getBg(void) { return _bg; }
		void setFg(XColor *xc) { _fg = xc; }
		void setBg(XColor *xc) { _bg = xc; }

		uint getFgAlpha(void) const { return _fg_alpha; }
		uint getBgAlpha(void) const { return _bg_alpha; }
		void setFgAlpha(uint alpha) { _fg_alpha = alpha; }
		void setBgAlpha(uint alpha) { _bg_alpha = alpha; }

		bool hasFg(void) const { return _fg != nullptr; }
		bool hasBg(void) const { return _bg != nullptr; }

	private:
		XColor *_fg;
		XColor *_bg;
		uint _fg_alpha;
		uint _bg_alpha;
	};

	PFont(void);
	virtual ~PFont(void);

	inline uint getJustify(void) const { return _justify; }

	inline void setJustify(uint j) { _justify = j; }
	inline void setOffset(uint x, uint y) { _offset_x = x; _offset_y = y; }

	int draw(Drawable dest, int x, int y, const std::string &text,
		 uint max_chars = 0, uint max_width = 0,
		 PFont::TrimType trim_type = FONT_TRIM_END);

	void trim(std::string &text, TrimType trim_type, uint max_width);
	void trimEnd(std::string &text, uint max_width);
	bool trimMiddle(std::string &text, uint max_width);

	static void setTrimString(const std::string &trim);

	uint justify(const std::string &text, uint max_width,
		     uint padding, uint chars);

	// virtual interface
	virtual bool load(const std::string& font_name) = 0;
	virtual void unload(void) { }

	virtual uint getWidth(const std::string& text, uint max_chars = 0) = 0;
	virtual uint getHeight(void) { return _height; }

	virtual void setColor(PFont::Color* color) = 0;

private:
	virtual void drawText(Drawable dest, int x, int y, const std::string &text,
			      uint chars, bool fg) = 0;

protected:
	uint _height, _ascent, _descent;
	uint _offset_x, _offset_y, _justify;

	static std::string _trim_string;
};

class PFontX11 : public PFont {
public:
	PFontX11(void);
	virtual ~PFontX11(void);

	// virtual interface
	virtual bool load(const std::string &name);
	virtual void unload(void);

	virtual uint getWidth(const std::string &text, uint max_chars = 0);

	virtual void setColor(PFont::Color *color);

private:
	virtual void drawText(Drawable dest, int x, int y, const std::string &text,
			      uint chars, bool fg);

private:
	XFontStruct *_font;
	GC _gc_fg, _gc_bg;
};

class PFontXmb : public PFont {
public:
	PFontXmb(void);
	virtual ~PFontXmb(void);

	// virtual interface
	virtual bool load(const std::string &name);
	virtual void unload(void);

	virtual uint getWidth(const std::string &text, uint max_chars = 0);

	virtual void setColor(PFont::Color *color);

private:
	virtual void drawText(Drawable dest, int x, int y, const std::string &text,
			      uint chars, bool fg);

	XFontSet _fontset;
	GC _gc_fg, _gc_bg;
	static const char *DEFAULT_FONTSET; /**< Default fallback fontset. */
};

#ifdef PEKWM_HAVE_XFT
class PFontXft : public PFont {
public:
	PFontXft(void);
	virtual ~PFontXft(void);

	// virtual interface
	virtual bool load(const std::string &font_name);
	virtual void unload(void);

	virtual uint getWidth(const std::string &text, uint max_chars = 0);

	virtual void setColor(PFont::Color *color);

private:
	virtual void drawText(Drawable dest, int x, int y, const std::string &text,
			      uint chars, bool fg);

	XftDraw *_draw;
	XftFont *_font;
	XftColor *_cl_fg, *_cl_bg;

	XRenderColor _xrender_color;
};
#endif // PEKWM_HAVE_XFT

#endif // _PEKWM_PFONT_HH_

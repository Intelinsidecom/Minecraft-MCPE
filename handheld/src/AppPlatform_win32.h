#ifndef APPPLATFORM_WIN32_H__
#define APPPLATFORM_WIN32_H__

#include "AppPlatform.h"
#include "platform/log.h"
#include "client/renderer/gles.h"
#include "world/level/storage/FolderMethods.h"
#include <png.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <windows.h>

bool static folderExists(const std::string& path)
{
	DWORD attribs = GetFileAttributesA(path.c_str());
	return (attribs != INVALID_FILE_ATTRIBUTES &&
		(attribs & FILE_ATTRIBUTE_DIRECTORY));
}

static void png_funcReadFile(png_structp pngPtr, png_bytep data, png_size_t length) {
	((std::istream*)png_get_io_ptr(pngPtr))->read((char*)data, length);
}

class AppPlatform_win32: public AppPlatform
{
public:
    AppPlatform_win32()
	:	_hwnd(NULL),
		_grabbed(false),
		_ignoreNextMove(false)
    {
    }
    
	void setWindowHandle(HWND hwnd) { _hwnd = hwnd; }
	HWND getWindowHandle() const { return _hwnd; }

	virtual void grabMouse();
	virtual void releaseMouse();
	bool isMouseGrabbed() const { return _grabbed; }

	bool shouldIgnoreNextMove() const { return _ignoreNextMove; }
	void setIgnoreNextMove(bool ignore) { _ignoreNextMove = ignore; }

	BinaryBlob readAssetFile(const std::string& filename) {
		FILE* fp = NULL;

		if (folderExists("/data")) {
			fp = fopen(("/data/" + filename).c_str(), "rb");
		}
		else if (folderExists("../../data")) {
			fp = fopen(("../../data/" + filename).c_str(), "rb");
		}

		if (!fp) {
			return BinaryBlob();
		}

		int size = getRemainingFileSize(fp);

		BinaryBlob blob;
		blob.size = size;
		blob.data = new unsigned char[size];

		fread(blob.data, 1, size, fp);
		fclose(fp);

		return blob;
	}

    void saveScreenshot(const std::string& filename, int glWidth, int glHeight) {
        //@todo
    }

    __inline unsigned int rgbToBgr(unsigned int p) {
        return (p & 0xff00ff00) | ((p >> 16) & 0xff) | ((p << 16) & 0xff0000);
    }

    TextureData loadTexture(const std::string& filename_, bool textureFolder)
	{
		TextureData out;
		std::string filename;
		if (folderExists("../../data")) {
			filename = "../../data/images/" + filename_;
		} else {
			filename = "data/images/" + filename_;
		}
		std::ifstream source(filename.c_str(), std::ios::binary);

		if (source) {
			png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

			if (!pngPtr)
				return out;

			png_infop infoPtr = png_create_info_struct(pngPtr);

			if (!infoPtr) {
				png_destroy_read_struct(&pngPtr, NULL, NULL);
				return out;
			}

			// Hack to get around the broken libpng for windows
			png_set_read_fn(pngPtr,(voidp)&source, png_funcReadFile);

			png_read_info(pngPtr, infoPtr);

			// Set up the texdata properties
			out.w = png_get_image_width(pngPtr, infoPtr);
			out.h = png_get_image_height(pngPtr, infoPtr);

			png_bytep* rowPtrs = new png_bytep[out.h];
			out.data = new unsigned char[4 * out.w * out.h];
			out.memoryHandledExternally = false;

			int rowStrideBytes = 4 * out.w;
			for (int i = 0; i < out.h; i++) {
				rowPtrs[i] = (png_bytep)&out.data[i*rowStrideBytes];
			}
			png_read_image(pngPtr, rowPtrs);

			// Teardown and return
			png_destroy_read_struct(&pngPtr, &infoPtr,(png_infopp)0);
			delete[] (png_bytep)rowPtrs;
			source.close();

			return out;
		}
		else
		{
			LOGI("Couldn't find file: %s\n", filename.c_str());
			return out;
		}
    }

    std::string getDateString(int s) {
        std::stringstream ss;
		ss << s << " s (UTC)";
		return ss.str();
	}

	virtual int checkLicense() {
		static int _z = 0;//20;
		_z--;
		if (_z < 0) return 0;
		//if (_z < 0) return 107;
		return -2;
	}

	virtual int getScreenWidth();
	virtual int getScreenHeight();

	virtual float getPixelsPerMillimeter();

	virtual bool supportsTouchscreen();
	virtual bool hasBuyButtonWhenInvalidLicense();

private:
	HWND _hwnd;
	bool _grabbed;
	bool _ignoreNextMove;
};

#endif /*APPPLATFORM_WIN32_H__*/

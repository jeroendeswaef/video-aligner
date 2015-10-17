#ifndef _DRAW_PARAMS_H
#define _DRAW_PARAMS_H

class DrawParams
{
public:
	DrawParams():
		width(0),
		height(0),
		pos(0) {}

	DrawParams(uint width, uint height, uint pos): 
		width(width),
		height(height),
		pos(pos) {}

	bool operator==(const DrawParams& rhs)
	{ 
		return(pos == rhs.pos && width == rhs.width && height == rhs.height);
	}

	bool operator!=(const DrawParams& rhs)
	{ 
		return!(this->operator==(rhs));
	}

	void setPos(uint pos) 
	{
		this->pos = pos;
	}

	uint getPos()
	{
		return pos;
	}

	void setDimensions(uint width, uint height) 
	{
		this->width = width;
		this->height = height;
	}

	uint getWidth() 
	{
		return width;
	}

	uint getHeight()
	{
		return height;
	}
	
private:
	uint width;
	uint height;
	uint pos;
};

#endif

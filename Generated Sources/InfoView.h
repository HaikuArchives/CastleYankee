/* InfoView.h */

#ifndef _H_InfoView_
#define _H_InfoView_

#include <View.h>

class InfoView : public BView {
public:
	InfoView(BRect frame);
	void	Draw(BRect updateRect);
	void	MouseDown(BPoint point);

protected:
	static const rgb_color	bgndColor;
};


#endif

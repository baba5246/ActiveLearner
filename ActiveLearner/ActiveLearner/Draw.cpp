
#include "Draw.h"


Draw::Draw(Mat src)
{
    if(!src.data)                                   // Check for invalid input
    {
        cout <<  "Could not open or find the image" << endl ;
    }
    else
    {
        namedWindow( "Display window", CV_WINDOW_AUTOSIZE );// Create a window for display.
        imshow( "Display window", src);                     // Show our image inside it.
        
        waitKey(0);
    }
}
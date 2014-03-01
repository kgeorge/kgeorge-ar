#if !defined(FIND_PATTERN_H_)
#define FIND_PATTERN_H_

struct PerFrameAppData;

void findPattern(
                 cv::Mat &capturedImage,
                 PerFrameAppData & perframeAppData);



#endif

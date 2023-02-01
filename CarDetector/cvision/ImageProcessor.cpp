#include "ImageProcessor.hpp"


namespace cvision
{
    PointMatrix ImageProcessor::getImgConvexHulls(cv::Mat& img)
    {
        PointMatrix contours;
        cv::findContours(img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        PointMatrix convexHulls(contours.size());
        for (unsigned int i = 0; i < contours.size(); i++) { cv::convexHull(contours[i], convexHulls[i]); }

        return convexHulls;
    }

    bool ImageProcessor::isMinimumObjSize(const MovingObjectPtr& obj)
    {
        // HARDCODED VALUES I KNOW...
        return (obj && obj->getArea() > 400 &&
            obj->getAspectRatio() > 0.2 && obj->getAspectRatio() < 4.0 &&
            obj->getWidth() > 30 && obj->getHeight() > 30 &&
            obj->getDiagonalSize() > 60.0 && obj->getContourArea() > 0.50);
    }

    MovingObjPtrList ImageProcessor::getMovingObjectsFromImg(cv::Mat& img)
    {
        auto convexHulls = getImgConvexHulls(img);
        MovingObjPtrList detectedMovingObjs;
        std::for_each(convexHulls.begin(), convexHulls.end(),
            [this, &detectedMovingObjs](auto& convexHull)
            {
                std::shared_ptr<MovingObject> possibleMovingObject =
                std::make_shared<MovingObject>(convexHull);
                if (isMinimumObjSize(possibleMovingObject)) { detectedMovingObjs.push_back(possibleMovingObject); }
            });

        return detectedMovingObjs;
    }

    cv::Mat ImageProcessor::preprocessImage(const cv::Mat& image) const
    {
        cv::Mat imageGrayScale, imageBlured, imageCanny;

        cv::cvtColor(image, imageGrayScale, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(imageGrayScale, imageBlured, cv::Size(5, 5), 0);

        return imageBlured;
    }

    cv::Mat ImageProcessor::getProcessedMergedImage(const cv::Mat& img1, const cv::Mat& img2) const
    {
        auto firstProcessedIamge = preprocessImage(img1);
        auto secondProcessedImage = preprocessImage(img2);
        cv::Mat imgDifference;
        cv::Mat imgThresh;
        cv::absdiff(firstProcessedIamge, secondProcessedImage, imgDifference);
        cv::threshold(imgDifference, imgThresh, 30, 255.0, cv::THRESH_BINARY);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        for (unsigned int i = 0; i < 2; i++) {
            cv::dilate(imgThresh, imgThresh, kernel);
            cv::dilate(imgThresh, imgThresh, kernel);
            cv::erode(imgThresh, imgThresh, kernel);
        }

        return imgThresh;
    }
} // namespace cvision

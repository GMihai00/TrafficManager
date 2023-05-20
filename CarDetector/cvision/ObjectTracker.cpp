#include "ObjectTracker.hpp"
#include <algorithm>
#include <float.h>

namespace cvision
{
    ObjectTracker::ObjectTracker(const uint16_t idCamera, IObserverPtr observer) :
        attachedObserver_(observer),
        camera_(idCamera),
        imageProcessor_(new ImageProcessor()),
        imageRender_(new ImageRender()),
        carDetector_(new CarDetect())
    {
    }

    ObjectTracker::ObjectTracker(const std::string videoPath, IObserverPtr observer) :
        attachedObserver_(observer),
        camera_(videoPath),
        imageProcessor_(new ImageProcessor()),
        imageRender_(new ImageRender()),
        carDetector_(new CarDetect())
    {

    }

    ObjectTracker::~ObjectTracker()
    {
        stopTracking();
    }

    void ObjectTracker::waitForFirstFrameApperance()
    {
        std::unique_lock<std::mutex> ulock(mutexProcess_);
        condVarProcess_.wait(ulock, [this] 
            {
                return ((!firstImageFrame_) &&
                    !imageQueue_.empty()) || !camera_.isRunning();
            });
        
        if (!camera_.isRunning())
            return;

        firstImageFrame_ = imageQueue_.pop();
        if (firstImageFrame_.value().empty())
        {
            LOG_ERR << "FIRST IMAGE WAS EMPTY!\n";
        }
        setupLines();
    }

    void ObjectTracker::processImages()
    {
        waitForFirstFrameApperance();
        while (camera_.isRunning() || !imageQueue_.empty())
        {
            std::unique_lock<std::mutex> ulock(mutexProcess_);
            condVarProcess_.wait(ulock, [this] { return !imageQueue_.empty() || !camera_.isRunning(); });

            if (!camera_.isRunning())
                continue;

            secondImageFrame_ = imageQueue_.pop();
            if (secondImageFrame_.value().empty())
            {
                LOG_ERR << "SECOND IMAGE WAS EMPTY!\n";
            }

            auto imgThreshold = imageProcessor_->getProcessedMergedImage(
                firstImageFrame_.value(),secondImageFrame_.value());
            /*   cv::imshow("TRESH", imgThreshold); // DISPLAY ONLY
            cv::waitKey(1);*/

            auto detectedMovingObjs = imageProcessor_->getMovingObjectsFromImg(imgThreshold);
            if (movingObjects_.empty()) 
            {
                std::for_each(detectedMovingObjs.begin(), detectedMovingObjs.end(),
                    [this](auto& movingObj) { addNewMovingObject(movingObj); });
            }
            else
            {
                matchFoundObjToExistingOnes(detectedMovingObjs);
            }

            auto cpy = secondImageFrame_.value().clone();
            drawRezultsOnImage(cpy);
            if (shouldRender_)
            {
                imageRender_->loadImage(cpy);
                imageRender_->startRendering();
            }
            detectedMovingObjs.clear();
            firstImageFrame_.value() = secondImageFrame_.value().clone();
            ulock.unlock();
        }
    }

    bool ObjectTracker::startTracking(bool shouldRender)
    {
        if (camera_.start())
        {
            shouldRender_ = shouldRender;
            threadCamera_ = std::thread([this]()
                {
                    while (camera_.isRunning())
                    {
                        // THIS IS A BUSY WAIT IT SHOULD BE CHANGED
                        // TRY TO CHANGE LOGIC HERE, TO HAVE THIS THINK WAITING,
                        // NOT THE CAMERA THREAD
                        // NEED TO INVESTIGATE ON HOW TO DO THIS
                        // TRIED SHARED_PTR ON MUTEX AND COND VARIABLE BUT 
                        // MUTEX SEEMS TO REMAIN A NULLPTR FOR SOME KIND OF REASON
                        if (imageQueue_.size() < 2)
                        {
                            std::unique_lock<std::mutex> ulock(mutexProcess_);
                            boost::optional<cv::Mat> image = camera_.getImage();
                            if (image != boost::none)
                            {
                                cv::Mat img = image.get();
                                imageQueue_.push(img);
                            }
                            ulock.unlock();
                            condVarProcess_.notify_one();
                        }
                    }
                    camera_.stop();
                });

            threadProcess_ = std::thread(std::bind(&ObjectTracker::processImages, this));
        }
        else
        {
            LOG_ERR << "Failed to start tracking";
            return false;
        }
        LOG_DBG << "Started Tracking";
        return true;
    }

    void ObjectTracker::stopTracking()
    {
        camera_.stop();

        carDetector_->stopDetecting();
        imageRender_->stopRendering();
        imageQueue_.clear();
        firstImageFrame_ = {};
        secondImageFrame_ = {};
        carCountLeft_ = 0;
        carCountRight_ = 0;

        condVarProcess_.notify_one();
        if (threadProcess_.joinable())
            threadProcess_.join();
        if (threadCamera_.joinable())
            threadCamera_.join();
    }

    void ObjectTracker::setupLines()
    {
        horizontalLinePosition_ = firstImageFrame_.value().rows / 2;

        crossingLineRight_[0].x = firstImageFrame_.value().cols / 2;
        crossingLineRight_[0].y = static_cast<int>(horizontalLinePosition_ * 0.75);
        crossingLineRight_[1].x = firstImageFrame_.value().cols - 1;
        crossingLineRight_[1].y = static_cast<int>(horizontalLinePosition_ * 0.75);

        crossingLineLeft_[0].x = 0;
        crossingLineLeft_[0].y = horizontalLinePosition_;
        crossingLineLeft_[1].x = firstImageFrame_.value().cols / 2 - 1;
        crossingLineLeft_[1].y = horizontalLinePosition_;
    }

    std::pair<MovingObjectGroupPtr, double> ObjectTracker::getClosestMovingObject(
            const MovingObjGroupPtrList& movingObjGroupList, const MovingObjectPtr& targetMovingObject)
    {
        MovingObjectGroupPtr closestMovingObjGroug;
        double leastDistance = DBL_MAX;
        std::for_each(movingObjGroupList.begin(), movingObjGroupList.end(),
            [this, &leastDistance, &closestMovingObjGroug, &targetMovingObject]
            (auto& movingObjGroup)
            {
                if (movingObjGroup->stillBeingTracked())
                {
                    if (movingObjGroup->getLastCenterPosition() != boost::none)
                    {
                        const auto& distance = 
                            distanceBetweenPoints(movingObjGroup->getLastCenterPosition().get(),
                                targetMovingObject->getCenter());
                        if (distance < leastDistance)
                        {
                            leastDistance = distance;
                            closestMovingObjGroug = movingObjGroup;
                        }
                    }
                }
            });
        return { closestMovingObjGroug, leastDistance };
    }

    void ObjectTracker::detectCarsInOjectGroup(const MovingObjectGroupPtr& objGroup)
    {
        const auto& objectImg = objGroup->getCroppedImage(secondImageFrame_.value());
        taskIdToObjGroup_[taskId_] = objGroup;
        carDetector_->loadTask(taskId_++, objectImg);
    }

    void ObjectTracker::matchFoundObjToExistingOnes(
        MovingObjPtrList& currentFrameMovingObj)
    {
        std::for_each(movingObjects_.begin(), movingObjects_.end(),
            [](auto& movingObjGroup)
            {
                    movingObjGroup->updateState(false);
            });

        std::for_each(currentFrameMovingObj.begin(), currentFrameMovingObj.end(),
            [this](std::shared_ptr<MovingObject>& movingObj)
            {
                auto closestGroupWithDistance = 
                    getClosestMovingObject(movingObjects_, movingObj);
                auto& closestMovingObjGroug = closestGroupWithDistance.first;
                double leastDistance = closestGroupWithDistance.second;
                if (leastDistance < movingObj->getDiagonalSize() * 0.5)
                {
                    closestMovingObjGroug->addMovingObject(movingObj);
                    closestMovingObjGroug->updateState(true);
                    if (closestMovingObjGroug->nrCars() == 0)
                    {
                        detectCarsInOjectGroup(closestMovingObjGroug);
                    }
                }
                else
                {
                    addNewMovingObject(movingObj);
                }
            });

        movingObjects_.remove_if([](auto& movingObjGroup) 
            { return (!movingObjGroup || (!movingObjGroup->stillBeingTracked()));});
    }

    void ObjectTracker::addNewMovingObject(const MovingObjectPtr& currentFrameMovingObj)
    {
        auto movingObjGroup = std::make_shared<MovingObjectGroup>();
        if (!movingObjGroup)
        {
            return;
        }
        movingObjGroup->addMovingObject(currentFrameMovingObj);
        movingObjGroup->updateState(true);
        movingObjects_.push_back(movingObjGroup);
        detectCarsInOjectGroup(movingObjGroup);
        carDetector_->startDetecting();
    }

    bool ObjectTracker::checkIfCarsCrossedRight()
    {
        bool objCrossedRight = false;
        MovingObjGroupPtrList objGroupToBeRemoved;
        for (const auto& movingObjGroup : movingObjects_) {
            size_t nrObjInGroup = movingObjGroup->getNrOfMovingObjInGroup();
            if (movingObjGroup->stillBeingTracked() == true 
                && nrObjInGroup >= 2) 
            {
                const auto previousPoz = movingObjGroup->getCenterPosition(nrObjInGroup - 2);
                const auto currentPoz = movingObjGroup->getCenterPosition(nrObjInGroup - 1);
                if (previousPoz && currentPoz && 
                    previousPoz.get().y > horizontalLinePosition_ * 0.75 &&
                        currentPoz.get().y <= horizontalLinePosition_ * 0.75 &&
                            currentPoz.get().x > (firstImageFrame_.value().cols) / 2 &&
                                currentPoz.get().x < firstImageFrame_.value().cols)
                {
                    size_t carsToAdd = movingObjGroup->nrCars();
                    if (carsToAdd != 0)
                    {
                        carCountRight_ += carsToAdd;
                        objCrossedRight = true;
                    }
                    else
                    {
                        objGroupToBeRemoved.push_back(movingObjGroup);
                    }
                }
            }
        }
            
        std::for_each(objGroupToBeRemoved.begin(), objGroupToBeRemoved.end(),
            [this](auto& movingObjGroup)
            {
                movingObjects_.remove(movingObjGroup);
            });

        return objCrossedRight;
    }

    bool ObjectTracker::checkIfCarsCrossedLeft()
    {  
        bool objCrossedRightLeft = false;
        MovingObjGroupPtrList objGroupToBeRemoved;
        for (const auto& movingObjGroup : movingObjects_) {
            size_t nrObjInGroup = movingObjGroup->getNrOfMovingObjInGroup();
            if (movingObjGroup->stillBeingTracked() == true
                && nrObjInGroup >= 2)
            {
                const auto previousPoz = movingObjGroup->getCenterPosition(nrObjInGroup - 2);
                const auto currentPoz = movingObjGroup->getCenterPosition(nrObjInGroup - 1);
                if (previousPoz && currentPoz &&
                    previousPoz.get().y <= horizontalLinePosition_ &&
                        currentPoz.get().y > horizontalLinePosition_ &&
                            currentPoz.get().x <= (firstImageFrame_.value().cols) / 2 && currentPoz.get().x >= 0)
                {
                    size_t carsToAdd = movingObjGroup->nrCars();
                    if (carsToAdd != 0)
                    {
                        carCountLeft_ += carsToAdd;
                        objCrossedRightLeft = true;
                    }
                    else
                    {
                        objGroupToBeRemoved.push_back(movingObjGroup);
                    }
                }
            }
        }

        std::for_each(objGroupToBeRemoved.begin(), objGroupToBeRemoved.end(),
            [this](auto& movingObjGroup)
            {
                movingObjects_.remove(movingObjGroup);
            });

        return objCrossedRightLeft;
    }

    void ObjectTracker::drawObjInfoOnImage(cv::Mat& img)
    {
        size_t cnt = 0;
        for (auto& movingObjGroup : movingObjects_)
        {
            auto movingObj = movingObjGroup->getLastState();
            if (movingObj)
            {
                if (movingObjGroup->nrCars() != 0)
                {
                    cv::rectangle(img, movingObj->getBoudingRect(), SCALAR_RED, 2);
                    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
                    double fontScale = (img.rows * img.cols) / 300000.0;
                    int fontThickness = (int)std::round(fontScale * 1.0);

                    cv::putText(img, std::to_string(cnt), movingObjGroup->getLastCenterPosition().get(),
                        fontFace, fontScale, SCALAR_GREEN, fontThickness);
                }
                else
                {
                    cv::rectangle(img, movingObj->getBoudingRect(), SCALAR_YELLOW, 2);
                    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
                    double fontScale = (img.rows * img.cols) / 300000.0;
                    int fontThickness = (int)std::round(fontScale * 1.0);

                    cv::putText(img, std::to_string(cnt), movingObjGroup->getLastCenterPosition().get(),
                        fontFace, fontScale, SCALAR_GREEN, fontThickness);
                }
            }
            cnt++;
        } 
    }

    void ObjectTracker::drawObjCountOnImage(cv::Mat& img)
    {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = (img.rows * img.cols) / 450000.0;
        int fontThickness = (int)std::round(fontScale * 2.5);

        cv::Size textSize = cv::getTextSize(std::to_string(carCountRight_),
            fontFace, fontScale, fontThickness, 0);
        cv::putText(img, "Total detected vehicles:" + std::to_string(carCountRight_), cv::Point(crossingLineRight_[0].x + 10, 25),
            fontFace, fontScale, SCALAR_RED, fontThickness);

        cv::Size textSize1 = cv::getTextSize(std::to_string(carCountLeft_),
            fontFace, fontScale, fontThickness, 0);
        cv::putText(img, "Total detected vehicles:" + std::to_string(carCountLeft_), cv::Point(crossingLineLeft_[0].x + 10, 25),
            fontFace, fontScale, SCALAR_YELLOW, fontThickness);
    }
        
    void ObjectTracker::drawRezultsOnImage(cv::Mat& img)
    {
        drawObjInfoOnImage(img);

        auto carCountMap = carDetector_->waitForFinish();
        
        for (const auto& rez : carCountMap)
        {
            const auto& taskId = rez.first;
            const auto& nrCars = rez.second;
            if (auto it = taskIdToObjGroup_.find(taskId); it != taskIdToObjGroup_.end())
            {
                if (it->second)
                {
                    it->second->updateCarState(nrCars);
                }
            }
        }

        taskIdToObjGroup_.clear();
        taskId_ = 0;

        cv::Scalar rightLaneColor = checkIfCarsCrossedRight() ? SCALAR_GREEN : SCALAR_RED;
        cv::line(img, crossingLineRight_[0], crossingLineRight_[1], rightLaneColor, 2);

        cv::Scalar leftLaneColor = checkIfCarsCrossedLeft() ? SCALAR_PINK : SCALAR_YELLOW;
        cv::line(img, crossingLineLeft_[0], crossingLineLeft_[1], leftLaneColor, 2);

        if (attachedObserver_ && (carCountLeft_ || carCountRight_))
        {
            attachedObserver_->notify();
        }

        drawObjCountOnImage(img);
    }

    std::pair<size_t, size_t> ObjectTracker::getCarCount()
    {
        return { carCountLeft_, carCountRight_ };
    }


    void ObjectTracker::subscribe(IObserverPtr observer)
    {
        attachedObserver_ = observer;
    }
} // namespace cvision
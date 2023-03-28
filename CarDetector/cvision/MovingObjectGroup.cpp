#include "MovingObjectGroup.hpp"

namespace cvision
{
    MovingObjectGroup::MovingObjectGroup() :
        objFoundInFrame_{ false },
        nrFramesWithoutMatch_{0},
        nrFramesWithoutBeeingCar_{ 0 },
        nrCars_{ 0 },
        sumCenterPoz_{ 0 }
    {
    }

    MovingObjectGroup::MovingObjectGroup(const MovingObjectGroup& cpy)
    {

    }

    void MovingObjectGroup::predictNextPosition()
    {
        // JUST FOR EASIER READING
        size_t n = centerPositions_.size();
        switch (n)
        {
        case 0:
            // NO ELEMENT ADDED SO CAN'T CALCULATE FUTURE
            break;
        case 1:
            futurePosition_.x = centerPositions_.back().x;
            futurePosition_.y = centerPositions_.back().y;
            break;
        default:
            const auto deltaX = (centerPositions_[n - 1].x * (n - 1) - sumCenterPoz_.x)
                / (((n * (n - 1)) / 2) * 1.0);
            const auto deltaY = (centerPositions_[n - 1].y * (n - 1) - sumCenterPoz_.y)
                / (((n * (n - 1)) / 2) * 1.0);
            futurePosition_.x = centerPositions_.back().x + deltaX;
            futurePosition_.y = centerPositions_.back().y + deltaY;
            break;
        }
	}

    void MovingObjectGroup::addMovingObject(const std::shared_ptr<MovingObject>& obj)
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (obj == nullptr)
        {
            // SHOULD NOT GET THERE
            return;
        }

        // TO NOT HAVE SO MANY UNNECESARY STATES STORED
        if (movingObjectStates_.size() == MAX_OBJECTS_STORED)
        {
            sumCenterPoz_ -= centerPositions_.front();
            movingObjectStates_.pop_front();
        }

        movingObjectStates_.push_back(obj);
        if (movingObjectStates_.size() > 1)
        {
            sumCenterPoz_ += centerPositions_.back();
        }
        centerPositions_.push_back(obj->getCenter());

        predictNextPosition();
            cntLock--;
    }

    void MovingObjectGroup::updateState(bool found)
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (found)
        {
            objFoundInFrame_ = true;
        }
        else
        {
            nrFramesWithoutMatch_++;
            objFoundInFrame_ = false;
        }
        cntLock--;
    }

    size_t MovingObjectGroup::getNrOfMovingObjInGroup()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        cntLock--;
        return this->centerPositions_.size();
    }

    bool MovingObjectGroup::stillBeingTracked()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        // HARDCODED VALUE BUT SHOULD BE FINE, IF NOT FOUND FOR 5 FRAMES
        // EITHER NOT MOVING OR PASSED CAMERA RANGE OF COVERAGE
        cntLock--;
        return ((nrFramesWithoutMatch_ < MAX_FRAMES_WITHOUT_A_MATCH) 
            && (nrFramesWithoutBeeingCar_ < MAX_FRAMES_WITHOUT_A_MATCH));
    }
    boost::optional<cv::Point> 
        MovingObjectGroup::getCenterPosition(const size_t& index)
    {
        cntLock++;
        std::scoped_lock lock(mutexGroup_);
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        if (index >= centerPositions_.size())
        {
            cntLock--;
            return boost::none;
        }
        cntLock--;
        return centerPositions_[index];
    }

    boost::optional<cv::Point> MovingObjectGroup::getLastCenterPosition()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (centerPositions_.size() == 0)
        {
            cntLock--;
            return boost::none;
        }
        cntLock--;
        return centerPositions_[centerPositions_.size() - 1];
    }

    cv::Point MovingObjectGroup::getFuturePosition()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        cntLock--;
        return futurePosition_;
    }

    boost::optional<double>  MovingObjectGroup::getDiagonalSize()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (movingObjectStates_.empty())
        {
            cntLock--;
            return boost::none;
        }
        cntLock--;
        return ((movingObjectStates_.begin())->get())->getDiagonalSize();
    }

    std::shared_ptr<MovingObject> MovingObjectGroup::getLastState()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        // NEED TO INVESTIGATE WHY I CAN NOT LOCK MUTEX
        if (movingObjectStates_.empty())
        {
            cntLock--;
            return nullptr;
        }
        cntLock--;
        return movingObjectStates_.back();
    }

    std::shared_ptr<MovingObject> MovingObjectGroup::getFirstState()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (movingObjectStates_.empty())
        {
            cntLock--;
            return nullptr;
        }
        cntLock--;
        return movingObjectStates_.front();
    }

    cv::Mat MovingObjectGroup::getCroppedImage(cv::Mat img)
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (movingObjectStates_.empty())
        {
            cntLock--;
            return img;
        }
        // THIS COULD BE THE STUPID CAUSE OF FAILURE...
        try
        {
            cv::Rect rect = movingObjectStates_.back()->getBoudingRect();
            rect.width *= 1.5;
            rect.height *= 1.5;
            rect.x -= 20;
            rect.y -= 20;
            cntLock--;
            return img(rect);
        }
        catch (std::exception ex)
        {
            cntLock--;
            return img(movingObjectStates_.back()->getBoudingRect());
        }
    }

    void MovingObjectGroup::updateCarState(uint8_t nrCars)
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        if (nrCars == 0)
        {
            nrFramesWithoutBeeingCar_++;
        }
        else
        {
            nrFramesWithoutBeeingCar_ = 0;
        }
        this->nrCars_ = nrCars;
        cntLock--;
    }

    uint8_t MovingObjectGroup::nrCars()
    {
        cntLock++;
        // std::cout << "CNT LOCKS: " << cntLock << "\n";
        std::scoped_lock lock(mutexGroup_);
        cntLock--;
        return nrCars_;
    }
} // namespace cvision

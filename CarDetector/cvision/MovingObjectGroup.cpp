#include "MovingObjectGroup.hpp"

namespace cvision
{
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
        assert(obj);
        std::scoped_lock lock(mutexGroup_);

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
            
    }

    void MovingObjectGroup::updateState(bool found)
    {
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
    }

    size_t MovingObjectGroup::getNrOfMovingObjInGroup()
    {
        std::scoped_lock lock(mutexGroup_);
        
        return this->centerPositions_.size();
    }

    bool MovingObjectGroup::stillBeingTracked()
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        // HARDCODED VALUE BUT SHOULD BE FINE, IF NOT FOUND FOR 5 FRAMES
        // EITHER NOT MOVING OR PASSED CAMERA RANGE OF COVERAGE
        
        return ((nrFramesWithoutMatch_ < MAX_FRAMES_WITHOUT_A_MATCH) 
            && (nrFramesWithoutBeeingCar_ < MAX_FRAMES_WITHOUT_A_MATCH));
    }
    boost::optional<cv::Point> 
        MovingObjectGroup::getCenterPosition(const size_t& index)
    {
        
        std::scoped_lock lock(mutexGroup_);
        
        if (index >= centerPositions_.size())
        {
            
            return boost::none;
        }
        
        return centerPositions_[index];
    }

    boost::optional<cv::Point> MovingObjectGroup::getLastCenterPosition()
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        if (centerPositions_.size() == 0)
        {
            
            return boost::none;
        }
        
        return centerPositions_[centerPositions_.size() - 1];
    }

    cv::Point MovingObjectGroup::getFuturePosition()
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        
        return futurePosition_;
    }

    boost::optional<double>  MovingObjectGroup::getDiagonalSize()
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        if (movingObjectStates_.empty())
        {
            
            return boost::none;
        }
        
        return ((movingObjectStates_.begin())->get())->getDiagonalSize();
    }

    std::shared_ptr<MovingObject> MovingObjectGroup::getLastState()
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        // NEED TO INVESTIGATE WHY I CAN NOT LOCK MUTEX
        if (movingObjectStates_.empty())
        {
            
            return nullptr;
        }
        
        return movingObjectStates_.back();
    }

    std::shared_ptr<MovingObject> MovingObjectGroup::getFirstState()
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        if (movingObjectStates_.empty())
        {
            
            return nullptr;
        }
        
        return movingObjectStates_.front();
    }

    cv::Mat MovingObjectGroup::getCroppedImage(cv::Mat img)
    {
        
        
        std::scoped_lock lock(mutexGroup_);
        if (movingObjectStates_.empty())
        {
            
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
            
            return img(rect);
        }
        catch (std::exception ex)
        {
            
            return img(movingObjectStates_.back()->getBoudingRect());
        }
    }

    void MovingObjectGroup::updateCarState(uint8_t nrCars)
    {
        
        
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
        
    }

    uint8_t MovingObjectGroup::nrCars()
    {
        std::scoped_lock lock(mutexGroup_);
        
        return nrCars_;
    }
} // namespace cvision

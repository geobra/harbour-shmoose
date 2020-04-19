#include "MucCollection.h"


MucCollection::MucCollection(boost::shared_ptr<Swift::MUC> muc, boost::shared_ptr<Swift::MUCBookmark> bookmark, std::string joinName):
    muc_(muc), bookmark_(bookmark), joinName_(joinName)
{

}

std::string MucCollection::getNickname() const
{
    return joinName_;
}

boost::shared_ptr<Swift::MUC> MucCollection::getMuc() const
{
    return muc_;
}

boost::shared_ptr<Swift::MUCBookmark> MucCollection::getBookmark() const
{
    return bookmark_;
}

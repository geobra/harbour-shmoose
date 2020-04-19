#ifndef MUCCOLLECTION_H
#define MUCCOLLECTION_H

#include <Swiften/Swiften.h>

class MucCollection
{
public:
    MucCollection(boost::shared_ptr<Swift::MUC> muc, boost::shared_ptr<Swift::MUCBookmark> bookmark, std::string joinName);

    std::string getNickname() const;
    boost::shared_ptr<Swift::MUC> getMuc() const;
    boost::shared_ptr<Swift::MUCBookmark> getBookmark() const;

private:
    MucCollection();

    boost::shared_ptr<Swift::MUC> muc_;
    boost::shared_ptr<Swift::MUCBookmark> bookmark_;
    std::string joinName_;

};

#endif // MUCCOLLECTION_H

#ifndef MUCCOLLECTION_H
#define MUCCOLLECTION_H

#include <Swiften/Swiften.h>

class MucCollection
{
public:
    MucCollection(std::shared_ptr<Swift::MUC> muc, std::shared_ptr<Swift::MUCBookmark> bookmark, std::string joinName);

    std::string getNickname() const;
    std::shared_ptr<Swift::MUC> getMuc() const;
    std::shared_ptr<Swift::MUCBookmark> getBookmark() const;

private:
    MucCollection();

    std::shared_ptr<Swift::MUC> muc_;
    std::shared_ptr<Swift::MUCBookmark> bookmark_;
    std::string joinName_;

};

#endif // MUCCOLLECTION_H

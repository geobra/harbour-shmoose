/*
 * Copyright (c) 2011 Tobias Markmann
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include "FileTransferController.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <Swiften/Base/Log.h>
#include <Swiften/Base/boost_bsignals.h>
#include <Swiften/FileTransfer/FileReadBytestream.h>
#include <Swiften/FileTransfer/FileTransferManager.h>
#include <Swiften/FileTransfer/OutgoingJingleFileTransfer.h>

#include <Swift/Controllers/Intl.h>
//#include <Swift/Controllers/UIInterfaces/ChatWindow.h>

#include <QDebug>

namespace Swift {

FileTransferController::FileTransferController(const JID& receipient, const std::string& filename, FileTransferManager* fileTransferManager) :
	sending(true), otherParty(receipient), filename(filename), ftManager(fileTransferManager), ftProgressInfo(0), /*chatWindow(0),*/ currentState(FileTransfer::State::WaitingForStart) {

}

FileTransferController::FileTransferController(IncomingFileTransfer::ref transfer) :
	sending(false), otherParty(transfer->getSender()), filename(transfer->getFileName()), transfer(transfer), ftManager(0), ftProgressInfo(0), /*chatWindow(0),*/ currentState(FileTransfer::State::WaitingForStart) {
	transfer->onStateChanged.connect(boost::bind(&FileTransferController::handleFileTransferStateChange, this, _1));
}

FileTransferController::~FileTransferController() {
	delete ftProgressInfo;
	transfer->onStateChanged.disconnect(boost::bind(&FileTransferController::handleFileTransferStateChange, this, _1));
}

const JID &FileTransferController::getOtherParty() const {
	return otherParty;
}

#if 0
std::string FileTransferController::setChatWindow(ChatWindow* wnd, std::string nickname) {
	chatWindow = wnd;
	if (sending) {
		uiID = wnd->addFileTransfer(QT_TRANSLATE_NOOP("", "me"), true, filename, boost::filesystem::file_size(boost::filesystem::path(filename)), "");
	} else {
		uiID = wnd->addFileTransfer(nickname, false, filename, transfer->getFileSizeInBytes(), transfer->getDescription());
	}
	return uiID;
}
#endif

void FileTransferController::setReceipient(const JID& receipient) {
	this->otherParty = receipient;
}

bool FileTransferController::isIncoming() const {
	return !sending;
}

FileTransfer::State FileTransferController::getState() const {
	return currentState;
}

int FileTransferController::getProgress() const {
	return ftProgressInfo ? ftProgressInfo->getPercentage() : 0;
}

boost::uintmax_t FileTransferController::getSize() const {
	if (transfer) {
		return transfer->getFileSizeInBytes();
	} else {
		return 0;
	}
}

void FileTransferController::start(std::string& description) {
	SWIFT_LOG(debug) << "FileTransferController::start" << std::endl;
	fileReadStream = boost::make_shared<FileReadBytestream>(boost::filesystem::path(filename));
	OutgoingFileTransfer::ref outgoingTransfer = ftManager->createOutgoingFileTransfer(otherParty, boost::filesystem::path(filename), description, fileReadStream);
	if (outgoingTransfer) {
		ftProgressInfo = new FileTransferProgressInfo(outgoingTransfer->getFileSizeInBytes());
		ftProgressInfo->onProgressPercentage.connect(boost::bind(&FileTransferController::handleProgressPercentageChange, this, _1));
		outgoingTransfer->onStateChanged.connect(boost::bind(&FileTransferController::handleFileTransferStateChange, this, _1));
		outgoingTransfer->onProcessedBytes.connect(boost::bind(&FileTransferProgressInfo::setBytesProcessed, ftProgressInfo, _1));
		outgoingTransfer->start();
		transfer = outgoingTransfer;
	} else {
		std::cerr << "File transfer not supported!" << std::endl;
	}
}

void FileTransferController::accept(std::string& file) {
	SWIFT_LOG(debug) << "FileTransferController::accept" << std::endl;
	IncomingFileTransfer::ref incomingTransfer = boost::dynamic_pointer_cast<IncomingFileTransfer>(transfer);
	if (incomingTransfer) {
		fileWriteStream = boost::make_shared<FileWriteBytestream>(boost::filesystem::path(file));

		ftProgressInfo = new FileTransferProgressInfo(transfer->getFileSizeInBytes());
		ftProgressInfo->onProgressPercentage.connect(boost::bind(&FileTransferController::handleProgressPercentageChange, this, _1));
		transfer->onProcessedBytes.connect(boost::bind(&FileTransferProgressInfo::setBytesProcessed, ftProgressInfo, _1));
		incomingTransfer->accept(fileWriteStream);
	} else {
		std::cerr << "Expected an incoming transfer in this situation!" << std::endl;
	}
}

void FileTransferController::cancel() {
	if (transfer) {
		transfer->cancel();
	} else {
		//chatWindow->setFileTransferStatus(uiID, ChatWindow::Canceled);
		qDebug() << "uid: " << QString::fromStdString(uiID) << ", Canceled";
	}
}

void FileTransferController::handleFileTransferStateChange(FileTransfer::State state) {
	currentState = state;
	onStateChanged();
	switch(state.type) {
		case FileTransfer::State::Initial:
			assert(false);
			return;
		case FileTransfer::State::Negotiating:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::Negotiating);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", Negotiating";
			return;
		case FileTransfer::State::Transferring:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::Transferring);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", Transferring";
			return;
		case FileTransfer::State::Canceled:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::Canceled);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", Canceled";
			return;
		case FileTransfer::State::Finished:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::Finished);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", Finished";
			if (fileWriteStream) {
				fileWriteStream->close();
			}
			return;
		case FileTransfer::State::Failed:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::FTFailed);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", FTFailed";
			return;
		case FileTransfer::State::WaitingForAccept:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::WaitingForAccept);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", WaitingForAccept";
			return;
		case FileTransfer::State::WaitingForStart:
			//chatWindow->setFileTransferStatus(uiID, ChatWindow::Initialisation);
			qDebug() << "uid: " << QString::fromStdString(uiID) << ", Initialisation";
			return;
	}
	assert(false);
}

void FileTransferController::handleProgressPercentageChange(int percentage) {
	onProgressChange();
	//chatWindow->setFileTransferProgress(uiID, percentage);
	qDebug() << "uid: " << QString::fromStdString(uiID) << ", %: " << percentage;
}

}

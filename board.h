#ifndef EMBEDDED_BOARD_H
#define EMBEDDED_BOARD_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QMap>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(BoMsg);

class BoardInfo
{
public:
	void printData();

	// Board

	QStringList m_name;
	QStringList m_vid;
	QStringList m_pid;

	// Upload

	QStringList m_upTool;
	QStringList m_upProtocol;
	QStringList m_upMaxSize;
	QStringList m_upMaxDataSize;
	QStringList m_upSpeed;
	QStringList m_upDisableFlush;
	QStringList m_upUse1k2bpsTouch;
	QStringList m_upWaitForUploadPort;

	// Boot loader

	QStringList m_blTool;
	QStringList m_blLowFuses;
	QStringList m_blHighFuses;
	QStringList m_blExtendedFuses;
	QStringList m_blFile;
	QStringList m_blNoblink;
	QStringList m_blUnlockBits;
	QStringList m_blLockBits;

	// Build

	QStringList m_bMcu;
	QStringList m_bFcpu;
	QStringList m_bVid;
	QStringList m_bPid;
	QStringList m_bUsbProduct;
	QStringList m_bBoard;
	QStringList m_bCore;
	QStringList m_bVariant;
	QStringList m_bExtraFlags;

	// Not an Option
	QStringList m_NaO;

};

/**
 * @brief A class to help deal with boards.txt
 *
 */
class Board
{
public:
	static  Board& instance();

	QMap<QString, BoardInfo> m_boards;

	QStringList m_boardList;
	QStringList m_boardNameList;
	void update();
	QString getIdFromName(QString _name);

private:
	Board& operator = (Board& other) = delete;
	Board(const Board& other) = delete;
	Board();

	void load();
	bool m_listed;
};

#endif // EMBEDDED_BOARD_H

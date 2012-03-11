#include <zimfilewrapper.h>
#include <QNetworkReply>
#include <QBuffer>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QPainter>
#include <QImage>
#include <QColor>

class ZimReply : public QNetworkReply
{
    Q_OBJECT
private:
    static ZimFileWrapper* zimFileWrapper;
public:
    ZimReply(QObject* object, const QNetworkRequest& request)
        : QNetworkReply(object)
        , position(0)
    {        
        setRequest(request);
        setOperation(QNetworkAccessManager::GetOperation);
        setHeader(QNetworkRequest::ContentTypeHeader,QVariant(QLatin1String("text/html")));
        open(ReadOnly|Unbuffered);
        setUrl(request.url());
        QString pattern = request.url().queryItemValue(QLatin1String("pattern"));
        Qt::BrushStyle brushStyle = Qt::SolidPattern;
        if (pattern == QLatin1String("dense"))
            brushStyle = Qt::Dense2Pattern;
        else if (pattern == QLatin1String("cross"))
            brushStyle = Qt::CrossPattern;
        else if (pattern == QLatin1String("diagonal"))
            brushStyle = Qt::FDiagPattern;
        const QString radiusString = request.url().queryItemValue(QLatin1String("radius"));
        const QString widthString = request.url().queryItemValue(QLatin1String("width"));
        const QString heightString = request.url().queryItemValue(QLatin1String("height"));
        const QString colorString = request.url().queryItemValue(QLatin1String("color"));
        QColor color(colorString);
        bool ok;
        int radius = radiusString.toInt(&ok);
        if (!ok)
            radius = 5;
        QSize size;
        size.setWidth(widthString.toInt(&ok));
        if (!ok)
            size.setWidth(100);
        size.setHeight(heightString.toInt(&ok));
        if (!ok)
            size.setHeight(100);
        connect(&watcher, SIGNAL(finished()), SLOT(readFromZimFileDone()));
        QFuture<QByteArray> future = QtConcurrent::run<QByteArray>(readFromZimFile, request.url());
        watcher.setFuture(future);
    }

    qint64 readData(char* data, qint64 maxSize)
    {
        const qint64 readSize = qMin(maxSize, (qint64)(buffer.size() - position));
        memcpy(data, buffer.constData() + position, readSize);
        position += readSize;
        return readSize;
    }

    virtual qint64 bytesAvailable() const
    {
        return buffer.size() - position;
    }

    virtual qint64 pos () const
    {
        return position;
    }

    virtual bool seek( qint64 pos )
    {
        if (pos < 0 || pos >= buffer.size())
            return false;
        position = pos;
        return true;
    }

    virtual qint64 size () const
    {
        return buffer.size();
    }


    static QByteArray readFromZimFile(const QUrl& url)
    {
        /*
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter painter(&image);
        QPainterPath path;
        path.addRoundedRect(QRectF(QPoint(0, 0), size), radius, radius);
        painter.fillPath(path, QBrush(color, style));
        QByteArray saveData;
        QBuffer b(&saveData);
        image.save(&b, "PNG");
        return saveData;*/

        QString text;
        if (ZimReply::zimFileWrapper->isValid()) {
            //TODO: path probably not correct
            text = ZimReply::zimFileWrapper->getArticleTextByUrl(url.path());
        } else {
            qDebug() << "Warning: Attempt to open article while no zim file open. Article URL: TODO";
            text = QLatin1String("<html><head></head><body>No zim file open</body></html>");
        }
        QByteArray saveData=text.toUtf8();
        return saveData;
    }

    static void setZimFileWrapper(ZimFileWrapper* zimFileWrapper) {
        ZimReply::zimFileWrapper = zimFileWrapper;
    }

    static ZimFileWrapper* getZimFileWrapper() {
        return zimFileWrapper;
    }

public slots:
    void readFromZimFileDone()
    {
        setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("text/html"));
        position = 0;
        buffer = watcher.result();
        emit readyRead();
        emit finished();
    }

    void abort()
    {
    }
public:
    QNetworkReply* rawReply;
private:
    QFutureWatcher<QByteArray> watcher;
    QByteArray buffer;
    int position;


};

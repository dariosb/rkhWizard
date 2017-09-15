#include <QtGui>
#include <QTranslator>

#include "definitions.h"
#include "rkhcfg.h"
#include "inputbool.h"
#include "inputstring.h"
#include "inputint.h"
#include "inputstring.h"
#include "inputstrlist.h"
#include "version.h"
#include "settingstree.h"
#include "settingdialog.h"

#include <QTreeWidget>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QScrollBar>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>

#ifdef WIN32
#include <windows.h>
#endif

#undef  SA
#define SA(x) QString::fromLatin1(x)
#define COMMENT_MAX_LONG 75
#define __DISABLE_NOPRINT_DEPEND__

RkhCfg::RkhCfg()
{
  m_treeWidget = new QTreeWidget;
  m_treeWidget->setColumnCount(1);
  m_topicStack = new QStackedWidget;
  m_inShowHelp = false;

  m_helper = new QTextEdit;
  m_helper->setObjectName("Help");
  m_helper->setReadOnly(true);
  m_helper->verticalScrollBar()->hide();
  m_splitter = new QSplitter(Qt::Vertical);
  m_splitter->addWidget(m_treeWidget);
  m_splitter->addWidget(m_helper);

  m_rightSide = new QWidget;
  QGridLayout *grid = new QGridLayout(m_rightSide);
  m_prev = new QPushButton(QIcon(QString::fromLatin1(":back")),tr("Previous"));
  m_prev->setEnabled(false);
  m_next = new QPushButton(QIcon(QString::fromLatin1(":next")),tr("Next"));
  grid->addWidget(m_topicStack,0,0,1,2);
  grid->addWidget(m_prev,1,0,Qt::AlignLeft);
  grid->addWidget(m_next,1,1,Qt::AlignRight);
  grid->setColumnStretch(0,1);
  grid->setRowStretch(0,1);

  m_splitter->setHidden(true);
  m_rightSide->setHidden(true);
  addWidget(m_splitter);
  addWidget(m_rightSide);
  connect(m_next,SIGNAL(clicked()),SLOT(nextTopic()));
  connect(m_prev,SIGNAL(clicked()),SLOT(prevTopic()));
  setMinimumSize(600,400);
}


RkhCfg::~RkhCfg()
{
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext())
  {
    i.next();
    delete i.value();
  }
}


QWidget *RkhCfg::createTopicWidget(QDomElement &elem)
{
  QScrollArea *area   = new QScrollArea;
#if 0
  QWidget     *topic  = new QWidget;
  QGridLayout *layout = new QGridLayout(topic);
  QDomElement child   = elem.firstChildElement();
  int row=0;
  while (!child.isNull())
  {
    QString type = child.attribute(SA("type"));
    if (type==SA("bool"))
    {
      InputBool *boolOption =
          new InputBool(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval"))==SA("1"),
            child.attribute(SA("docs"))
           );
      m_options.insert(
          child.attribute(SA("id")),
          boolOption
         );
      connect(boolOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(boolOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("string"))
    {
      InputString::StringMode mode;
      QString format = child.attribute(SA("format"));
      if (format==SA("dir"))
      {
        mode = InputString::StringDir;
      }
      else if (format==SA("file"))
      {
        mode = InputString::StringFile;
      }
      else // format=="string"
      {
        mode = InputString::StringFree;
      }
      InputString *stringOption =
          new InputString(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval")),
            mode,
            child.attribute(SA("docs")),
            child.attribute(SA("abspath"))
           );
      m_options.insert(
          child.attribute(SA("id")),
          stringOption
         );
      connect(stringOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(stringOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("enum"))
    {
      InputString *enumList = new InputString(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval")),
            InputString::StringFixed,
            child.attribute(SA("docs"))
           );
      QDomElement enumVal = child.firstChildElement();
      while (!enumVal.isNull())
      {
        enumList->addValue(enumVal.attribute(SA("name")));
        enumVal = enumVal.nextSiblingElement();
      }
      enumList->setDefault();

      m_options.insert(child.attribute(SA("id")),enumList);
      connect(enumList,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(enumList,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("int"))
    {
      InputInt *intOption =
          new InputInt(
            layout,row,
            child.attribute(SA("id")),
            child.attribute(SA("defval")).toInt(),
            child.attribute(SA("minval")).toInt(),
            child.attribute(SA("maxval")).toInt(),
            child.attribute(SA("docs"))
          );
      m_options.insert(
          child.attribute(SA("id")),
          intOption
        );
      connect(intOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(intOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("list"))
    {
      InputStrList::ListMode mode;
      QString format = child.attribute(SA("format"));
      if (format==SA("dir"))
      {
        mode = InputStrList::ListDir;
      }
      else if (format==SA("file"))
      {
        mode = InputStrList::ListFile;
      }
      else if (format==SA("filedir"))
      {
        mode = InputStrList::ListFileDir;
      }
      else // format=="string"
      {
        mode = InputStrList::ListString;
      }
      QStringList sl;
      QDomElement listVal = child.firstChildElement();
      while (!listVal.isNull())
      {
        sl.append(listVal.attribute(SA("name")));
        listVal = listVal.nextSiblingElement();
      }
      InputStrList *listOption =
          new InputStrList(
            layout,row,
            child.attribute(SA("id")),
            sl,
            mode,
            child.attribute(SA("docs"))
          );
      m_options.insert(
          child.attribute(SA("id")),
          listOption
        );
      connect(listOption,SIGNAL(showHelp(Input*)),SLOT(showHelp(Input*)));
      connect(listOption,SIGNAL(changed()),SIGNAL(changed()));
    }
    else if (type==SA("obsolete"))
    {
      // ignore
    }
    else // should not happen
    {
      printf("Unsupported type %s\n",qPrintable(child.attribute(SA("type"))));
    }
    child = child.nextSiblingElement();
  }

  // compute dependencies between options
  child = elem.firstChildElement();
  while (!child.isNull())
  {
    QString dependsOn = child.attribute(SA("depends"));
    QString id        = child.attribute(SA("id"));
    Input *parentOption;
    if (dependsOn.isEmpty())
    {
        child = child.nextSiblingElement();
        continue;
    }

    DTree *dt = new DTree(dependsOn,&m_options);
    Input *thisOption   = m_options[id];
    foreach(QString po, dt->depencies)
    {
        parentOption = m_options[po];
        if (parentOption && thisOption)
        {
            parentOption->addDependencyDTree(dt);
            parentOption->addDependency(thisOption);
        }
    }

    child = child.nextSiblingElement();
  }

  // set initial dependencies
  QHashIterator<QString,Input*> i(m_options);
  while (i.hasNext())
  {
    i.next();
    if (i.value())
    {
      i.value()->updateDependencies(i.value()->value());
    }
  }

  layout->setRowStretch(row,1);
  layout->setColumnStretch(1,2);
  layout->setSpacing(5);
  topic->setLayout(layout);
  area->setWidget(topic);
  area->setWidgetResizable(true);
#endif
  return area;
}


void RkhCfg::createTopics(const QDomElement &rootElem)
{
    QList<QTreeWidgetItem*> items;
    QDomElement childElem = rootElem.firstChildElement();
    m_treeWidget->clear();
    QString name;

    if(childElem.tagName() != SA("compounddef") )
        return;

    childElem = childElem.firstChildElement();
    name = childElem.tagName();
    while (!childElem.isNull())
    {
      if (childElem.tagName()==SA("innergroup"))
      {
        QString name = childElem.text();
        name = name.remove(0, strlen("Related to "));
        name = name.toUpper();
        items.append(new QTreeWidgetItem((QTreeWidget*)0,QStringList(name)));
        QWidget *widget = createTopicWidget(childElem);
        m_topics[name] = widget;
        m_topicStack->addWidget(widget);
      }
      childElem = childElem.nextSiblingElement();
      name = childElem.tagName();
    }
    m_treeWidget->setHeaderLabels(QStringList() << SA("Topics"));
    m_treeWidget->insertTopLevelItems(0,items);
    connect(m_treeWidget,
            SIGNAL(currentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)),
            this,
            SLOT(activateTopic(QTreeWidgetItem *,QTreeWidgetItem *)));
}

void RkhCfg::loadXmlFile(QString xmlFile)
{
    QFile file(xmlFile);
    QString err;
    int errLine,errCol;
    xmlDoc.clear();

    if (file.open(QIODevice::ReadOnly))
    {
      if (!xmlDoc.setContent(&file,false,&err,&errLine,&errCol))
      {
        QString msg = tr("Error parsing internal config.xml at line %1 column %2.\n%3").
                    arg(errLine).arg(errCol).arg(err);
        QMessageBox::warning(this, tr("Error"), msg);
        exit(1);
      }
    }
    m_rootElement = xmlDoc.documentElement();

    m_rightSide->setHidden(false);
    m_splitter->setHidden(false);

    createTopics(m_rootElement);

}


void
RkhCfg::loadDefaultRkhConfig(void)
{
  loadXmlFile(RKH_CFG_FILE);
}


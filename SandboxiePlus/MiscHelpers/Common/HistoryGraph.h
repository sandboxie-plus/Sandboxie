#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CHistoryGraph: public QObject
{
public:
	CHistoryGraph(bool bSimpleMode = false, QColor BkG = Qt::white, QObject* parent = NULL) : QObject(parent) {
		m_SimpleMode = bSimpleMode;
		m_BkG = BkG;
	}
	~CHistoryGraph() {}

	void AddValue(int Id, QColor Color)
	{
		m_Values.insert(Id, { 0.0, Color });
	}

	void SetValue(int Id, float Value)
	{
		m_Values[Id].Value = Value;
	}

	void Update(int CellHeight, int CellWidth)
	{
		Update(m_Graph, m_BkG, m_Values, CellHeight, CellWidth, m_SimpleMode);
	}

	QImage GetImage() const { return m_Graph; }

	struct SValue
	{
		float Value;
		QColor Color;
	};

	static void Update(QImage& m_Graph, const QColor& m_BkG, const QMap<int, SValue>& m_Values, int CellHeight, int CellWidth, bool m_SimpleMode = true)
	{
		// init / resize
		if(m_Graph.height() != CellWidth /*|| m_Graph.width() != curHeight*/)
		{
			QImage Graph = QImage(CellHeight, CellWidth, QImage::Format_RGB32);
			QPainter qp(&Graph);
			qp.fillRect(-1, -1, CellHeight+1, CellWidth+1, m_BkG);
			if (!m_Graph.isNull())
				qp.drawImage(0, Graph.height() - m_Graph.height(), m_Graph);
			m_Graph = Graph;
		}

		// shift
		uchar *dest = m_Graph.bits();
		int pos = m_Graph.sizeInBytes() - m_Graph.bytesPerLine();
		memmove(dest, dest + m_Graph.bytesPerLine(), pos);
		QPainter qp(&m_Graph);

		// draw new data points
		int top = 0;

#if 0
		foreach(const SValue& Value, m_Values)
		{
			int x = (float)(m_Graph.width()) * Value.Value;
			if (x > top)
				top = x;

			qp.setPen(Value.Color);
			qp.drawLine(0, m_Graph.height() - 1, x, m_Graph.height());
		}
			
		qp.setPen(Qt::white);
		qp.drawLine(top, m_Graph.height() - 1, m_Graph.width()-1, m_Graph.height());
#else
		dest += pos;
		memset(dest, 0, m_Graph.bytesPerLine()); // fill line black
		ASSERT(m_Graph.depth() == 32);
		int max = m_Graph.width();
		//ASSERT(max * 4 == m_Graph.bytesPerLine());
		
		foreach(const SValue& Value, m_Values)
		{
			int x = (float)(max) * Value.Value;
			if (x > max)
				x = max;
			if (x > top)
				top = x;
			x *= 4;

			int r, g, b;
			Value.Color.getRgb(&r, &g, &b);
			

			for (int i = 0; i < x; i += 4)
			{
				if (m_SimpleMode || *(quint32*)(dest + i) == 0)
				{
					dest[i    ] = b;
					dest[i + 1] = g;
					dest[i + 2] = r;
				}
				else
				{
#define DIV 4/5
					int cb = (int)dest[i    ]*DIV + b*DIV;
					int cg = (int)dest[i + 1]*DIV + g*DIV;
					int cr = (int)dest[i + 2]*DIV + r*DIV;

					dest[i    ] = qMin(cb, 255);
					dest[i + 1] = qMin(cg, 255);
					dest[i + 2] = qMin(cr, 255);
				}
			}
		}

		// fill whats left of the line
		/*top *= 4;
		if (top < m_Graph.bytesPerLine()) // fill rest white
			memset(dest + top, 0xFF, m_Graph.bytesPerLine() - top);*/
		{
			int r, g, b;
			m_BkG.getRgb(&r, &g, &b);

			top *= 4;
			max *= 4;
			for (int i = top; i < max; i += 4)
			{
				dest[i] = b;
				dest[i + 1] = g;
				dest[i + 2] = r;
			}
		}
#endif
	}

protected:
	QImage				m_Graph;
	QColor				m_BkG;
	QMap<int, SValue>	m_Values;
	bool				m_SimpleMode;
};

class CHistoryWidget: public QWidget
{
public:
	CHistoryWidget(CHistoryGraph* pHistoryGraph, QWidget* parent = NULL) : QWidget(parent) { m_pHistoryGraph = pHistoryGraph; }
	~CHistoryWidget() {}

protected:
	void paintEvent(QPaintEvent* e)
	{
		if (m_pHistoryGraph)
		{
			QImage HistoryGraph = m_pHistoryGraph->GetImage();
			QPainter qp(this);
			qp.translate(width() - HistoryGraph.height() - 1, height());
			qp.rotate(270);
			qp.drawImage(0, 0, HistoryGraph);
		}
	}

	QPointer<CHistoryGraph> m_pHistoryGraph;
};
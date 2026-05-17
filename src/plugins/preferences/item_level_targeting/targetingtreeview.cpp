#include "targetingtreeview.h"

#include <QAbstractItemModel>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPainter>
#include <QPalette>

namespace preferences
{

namespace
{
constexpr int kEdgeBand = 5;
}

TargetingTreeView::TargetingTreeView(QWidget *parent)
    : QTreeView(parent)
{}

TargetingTreeView::DropFeedbackKind TargetingTreeView::dropFeedbackKind() const
{
    return m_feedbackKind;
}

QModelIndex TargetingTreeView::dropFeedbackIndex() const
{
    return m_feedbackIndex;
}

bool TargetingTreeView::updateDropFeedbackFor(const QPoint &pos, const QMimeData *mimeData,
                                              Qt::DropAction action)
{
    const DropProposal proposal = proposalFor(pos);
    const bool valid = proposal.kind != DropFeedbackKind::None
        && model()
        && model()->canDropMimeData(mimeData, action, proposal.row, 0, proposal.parent);
    if (!valid)
    {
        clearFeedback();
        return false;
    }

    setFeedback(proposal.kind, proposal.index);
    return true;
}

void TargetingTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (!updateDropFeedbackFor(event->pos(), event->mimeData(), event->proposedAction()))
    {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
}

void TargetingTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    clearFeedback();
    QTreeView::dragLeaveEvent(event);
}

void TargetingTreeView::dropEvent(QDropEvent *event)
{
    const DropProposal proposal = proposalFor(event->pos());
    const bool accepted = proposal.kind != DropFeedbackKind::None
        && model()
        && model()->dropMimeData(event->mimeData(), event->proposedAction(),
                                 proposal.row, 0, proposal.parent);
    clearFeedback();
    if (accepted)
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void TargetingTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
    if (m_feedbackKind == DropFeedbackKind::None || !m_feedbackIndex.isValid())
    {
        return;
    }

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing, false);
    const QRect rect = visualRect(m_feedbackIndex);
    const QColor accent = palette().color(QPalette::Highlight);

    if (m_feedbackKind == DropFeedbackKind::IntoCollection)
    {
        QColor fill = accent;
        fill.setAlpha(36);
        painter.fillRect(rect.adjusted(0, 1, -1, -1), fill);
        painter.setPen(QPen(accent, 2));
        painter.drawRect(rect.adjusted(1, 1, -2, -2));
        return;
    }

    const int y = m_feedbackKind == DropFeedbackKind::InsertBefore
        ? rect.top()
        : rect.bottom() + 1;
    painter.setPen(QPen(accent, 2));
    painter.drawLine(rect.left(), y, viewport()->width() - 1, y);
}

TargetingTreeView::DropProposal TargetingTreeView::proposalFor(const QPoint &pos) const
{
    DropProposal proposal;
    if (!model())
    {
        return proposal;
    }

    const QModelIndex index = indexAt(pos);
    if (!index.isValid())
    {
        proposal.kind   = DropFeedbackKind::InsertAfter;
        if (model()->rowCount() > 0)
        {
            proposal.index = model()->index(model()->rowCount() - 1, 0);
        }
        proposal.parent = QModelIndex();
        proposal.row    = model()->rowCount();
        return proposal;
    }

    const QRect rect = visualRect(index);
    proposal.index = index;
    proposal.parent = index.parent();

    if (pos.y() <= rect.top() + kEdgeBand)
    {
        proposal.kind = DropFeedbackKind::InsertBefore;
        proposal.row  = index.row();
    }
    else if (pos.y() >= rect.bottom() - kEdgeBand)
    {
        proposal.kind = DropFeedbackKind::InsertAfter;
        proposal.row  = index.row() + 1;
    }
    else
    {
        proposal.kind   = DropFeedbackKind::IntoCollection;
        proposal.parent = index;
        proposal.row    = -1;
    }
    return proposal;
}

void TargetingTreeView::setFeedback(DropFeedbackKind kind, const QModelIndex &index)
{
    if (m_feedbackKind == kind && m_feedbackIndex == index)
    {
        return;
    }
    m_feedbackKind  = kind;
    m_feedbackIndex = index;
    viewport()->update();
}

void TargetingTreeView::clearFeedback()
{
    if (m_feedbackKind == DropFeedbackKind::None && !m_feedbackIndex.isValid())
    {
        return;
    }
    m_feedbackKind = DropFeedbackKind::None;
    m_feedbackIndex = QModelIndex();
    viewport()->update();
}

} // namespace preferences

#include "targetingtreeview.h"

#include <QAbstractItemModel>
#include <QDragEnterEvent>
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

Qt::DropAction effectiveDropAction(const QDropEvent *event)
{
    return event->possibleActions().testFlag(Qt::MoveAction)
        ? Qt::MoveAction
        : event->proposedAction();
}
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
    DropProposal proposal;
    if (!resolveDropProposal(pos, mimeData, action, proposal))
    {
        clearFeedback();
        return false;
    }

    setFeedback(proposal.kind, proposal.index);
    return true;
}

void TargetingTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    const auto action = effectiveDropAction(event);
    if (!updateDropFeedbackFor(event->pos(), event->mimeData(), action))
    {
        event->ignore();
        return;
    }

    event->setDropAction(action);
    event->accept();
}

void TargetingTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    const auto action = effectiveDropAction(event);
    if (action != Qt::MoveAction || !model())
    {
        event->ignore();
        return;
    }

    event->setDropAction(action);
    event->accept();
}

void TargetingTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    clearFeedback();
    QTreeView::dragLeaveEvent(event);
}

void TargetingTreeView::dropEvent(QDropEvent *event)
{
    const auto action = effectiveDropAction(event);
    DropProposal proposal;
    const bool accepted = resolveDropProposal(event->pos(), event->mimeData(), action, proposal)
        && model()
        && model()->dropMimeData(event->mimeData(), action, proposal.row, 0, proposal.parent);
    clearFeedback();
    if (accepted)
    {
        event->setDropAction(action);
        event->accept();
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

TargetingTreeView::DropProposal TargetingTreeView::insertionProposalFor(
    const QModelIndex &index, const QPoint &pos) const
{
    DropProposal proposal;
    if (!index.isValid())
    {
        return proposal;
    }

    const QRect rect = visualRect(index);
    proposal.index = index;
    proposal.parent = index.parent();
    if (pos.y() < rect.center().y())
    {
        proposal.kind = DropFeedbackKind::InsertBefore;
        proposal.row = index.row();
    }
    else
    {
        proposal.kind = DropFeedbackKind::InsertAfter;
        proposal.row = index.row() + 1;
    }
    return proposal;
}

bool TargetingTreeView::resolveDropProposal(const QPoint &pos,
                                            const QMimeData *mimeData,
                                            Qt::DropAction action,
                                            DropProposal &proposal) const
{
    if (!model())
    {
        proposal = {};
        return false;
    }

    proposal = proposalFor(pos);
    auto canDrop = [this, mimeData, action](const DropProposal &candidate) {
        return candidate.kind != DropFeedbackKind::None
            && model()->canDropMimeData(mimeData, action, candidate.row, 0, candidate.parent);
    };
    if (canDrop(proposal))
    {
        return true;
    }

    if (proposal.kind == DropFeedbackKind::IntoCollection && proposal.index.isValid())
    {
        const DropProposal insertion = insertionProposalFor(proposal.index, pos);
        if (canDrop(insertion))
        {
            proposal = insertion;
            return true;
        }
    }

    proposal = {};
    return false;
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

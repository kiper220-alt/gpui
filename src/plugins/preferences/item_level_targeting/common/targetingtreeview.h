#ifndef GPUI_TARGETING_TREE_VIEW_H
#define GPUI_TARGETING_TREE_VIEW_H

#include <QTreeView>

namespace preferences
{

//! Tree view that renders explicit feedback for targeting drag-and-drop.
class TargetingTreeView : public QTreeView
{
public:
    enum class DropFeedbackKind
    {
        None,
        InsertBefore,
        InsertAfter,
        IntoCollection,
    };

    explicit TargetingTreeView(QWidget *parent = nullptr);

    DropFeedbackKind dropFeedbackKind() const;
    QModelIndex dropFeedbackIndex() const;
    bool updateDropFeedbackFor(const QPoint &pos, const QMimeData *mimeData,
                               Qt::DropAction action);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    struct DropProposal
    {
        DropProposal()
            : kind(DropFeedbackKind::None)
            , index()
            , parent()
            , row(-1)
        {}

        DropFeedbackKind kind;
        QModelIndex index;
        QModelIndex parent;
        int row;
    };

    DropProposal proposalFor(const QPoint &pos) const;
    DropProposal insertionProposalFor(const QModelIndex &index, const QPoint &pos) const;
    bool resolveDropProposal(const QPoint &pos,
                             const QMimeData *mimeData,
                             Qt::DropAction action,
                             DropProposal &proposal) const;
    void setFeedback(DropFeedbackKind kind, const QModelIndex &index);
    void clearFeedback();

    DropFeedbackKind m_feedbackKind;
    QModelIndex m_feedbackIndex;
};

} // namespace preferences

#endif // GPUI_TARGETING_TREE_VIEW_H

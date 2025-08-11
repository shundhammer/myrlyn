/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) 2024-25 SUSE LLC
    License:  GPL V2 - See file LICENSE for details.

 */


#ifndef TreeItem_h
#define TreeItem_h

#include <string>


/**
 * Template class for tree items that can handle tree children in a
 * generic way - firstChild(), next() and parent(). Each item stores one value
 * of type 'PAYLOAD'.
 *
 * Class 'PAYLOAD' needs to provide operator=().
 **/
template<class PAYLOAD> class TreeItem
{
public:

    /**
     * Constructor. Creates a new tree item with value "val" and inserts it
     * ( without maintaining any meaningful sort order! ) into the children list
     * of "parent".
     **/
    TreeItem( const PAYLOAD &     val,
              TreeItem<PAYLOAD> * parent = 0 )
        : _value( val )
        , _parent( parent )
        , _next(0)
        , _firstChild(0)
    {
        if ( _parent )
            _parent->addChild( this );
    }


protected:

    /**
     * Constructor to be called for derived classes: Decide whether or not to
     * automatically insert this item into the parent's children list. Useful
     * for derived classes that want to maintain a specific sort order among
     * children.
     **/
    TreeItem( PAYLOAD             val,
              bool                autoAddChild,
              TreeItem<PAYLOAD> * parent = 0 )
        : _value( val )
        , _parent( parent )
        , _next(0)
        , _firstChild(0)
    {
        if ( _parent && autoAddChild )
            _parent->addChild( this );
    }


private:
    /**
     * Private ( i.e. disabled ) copy constructor and operator=()
     * - neither makes any sense with this class.
     **/
    TreeItem             ( const TreeItem<PAYLOAD> & ) {}
    TreeItem & operator= ( const TreeItem<PAYLOAD> & ) {}


public:

    /**
     * Destructor. Takes care of children - they will be deleted along with
     * this item.
     **/
    virtual ~TreeItem()
    {
        TreeItem<PAYLOAD> * child = firstChild();

        while ( child )
        {
            TreeItem<PAYLOAD> * lastChild = child;
            child = child->next();
            delete lastChild;
        }
    }


    /**
     * Returns this item's value, the "payload".
     **/
    const PAYLOAD & value() const { return _value; }

    /**
     * Set this item's value, the "payload".
     *
     * If the sort order among children of one level is important, overwrite
     * this method and change the sort order according to the new value.
     * The template class itself never calls this.
     **/
    void setValue( PAYLOAD newValue ) { _value = newValue; }

    /**
     * Returns this item's parent or 0 if there is none.
     **/
    TreeItem<PAYLOAD> * parent()     const { return _parent; }

    /**
     * Returns this item's next sibling or 0 if there is none.
     **/
    TreeItem<PAYLOAD> * next()       const { return _next; }

    /**
     * Returns this item's first child or 0 if there is none.
     **/
    TreeItem<PAYLOAD> * firstChild() const { return _firstChild; }

    /**
     * Sets this item's parent.
     **/
    void setParent( TreeItem<PAYLOAD> * newParent ) { _parent = newParent;  }

    /**
     * Sets this item's next sibling.
     **/
    void setNext( TreeItem<PAYLOAD> *   newNext )   { _next = newNext; }

    /**
     * Sets this item's first child.
     **/
    void setFirstChild( TreeItem<PAYLOAD> * newFirstChild )
        { _firstChild = newFirstChild; }


    /**
     * Add a child to the internal children list - usually called from within
     * the child's default constructor.
     *
     * This default method does not maintain any meaningful sorting order -
     * derived classes that require this might want to use the other
     * constructor ( with 'autoAddChild' set to 'false' ) take care of child
     * insertion themselves.
     **/
    void addChild( TreeItem<PAYLOAD> * newChild )
    {
        if ( newChild )
        {
            newChild->setNext( firstChild() );
            setFirstChild( newChild );
        }
    }


protected:

    PAYLOAD             _value;
    TreeItem<PAYLOAD> * _parent;
    TreeItem<PAYLOAD> * _next;
    TreeItem<PAYLOAD> * _firstChild;
};



/**
 * Template class for tree items that maintain sort order.
 *
 * Class 'PAYLOAD' to provide operator<() in addition to what template
 *'TreeItem' requires.
 **/
template<class PAYLOAD> class SortedTreeItem: public TreeItem<PAYLOAD>
{
public:

    /**
     * Constructor. Creates a new tree item with value "val" and inserts it in
     * ascending sort order into the children list of "parent".
     **/
    SortedTreeItem( PAYLOAD                    val,
                    SortedTreeItem<PAYLOAD> *  parentItem = 0 )
        : TreeItem<PAYLOAD> ( val, false, parentItem )
    {
        if ( parentItem )
        {
            // Hopefully we have a SortedTreeItem parent
            SortedTreeItem<PAYLOAD> * sortParent =
                dynamic_cast<SortedTreeItem<PAYLOAD> *> ( parentItem );

            if ( sortParent )
                sortParent->insertChildSorted( this );
            else // no SortedTreeItem parent - add unsorted
                parentItem->addChild( this );
        }
    }


    /**
     * Destructor.
     **/
    virtual ~SortedTreeItem() {}


    /**
     * Insert a child into the internal children list in ascending sort order.
     * Called from the new child's constructor, thus 'public'.
     **/
    void insertChildSorted( SortedTreeItem<PAYLOAD> * newChild )
    {
        if ( ! newChild )
            return;

        if ( ! firstChild() ||
             newChild->value() < firstChild()->value() )
        {
            // Insert as first child

            newChild->setNext( firstChild() );
            this->setFirstChild( newChild );
        }
        else
        {
            // Search correct place to insert

            TreeItem<PAYLOAD> * child = firstChild();

            while ( child->next() &&
                    child->next()->value() < newChild->value() )
            {
                child = child->next();
            }


            // Insert after 'child'

            newChild->setNext( child->next() );
            child->setNext( newChild );
        }
    }


    /**
     * Returns this item's parent or 0 if there is none.
     **/
    SortedTreeItem<PAYLOAD> *   parent()        const
        { return ( SortedTreeItem<PAYLOAD> * ) TreeItem<PAYLOAD>::_parent; }

    /**
     * Returns this item's next sibling or 0 if there is none.
     **/
    SortedTreeItem<PAYLOAD> *   next()          const
        { return ( SortedTreeItem<PAYLOAD> * ) TreeItem<PAYLOAD>::_next; }

    /**
     * Returns this item's first child or 0 if there is none.
     **/
    SortedTreeItem<PAYLOAD> *   firstChild()    const
        { return ( SortedTreeItem<PAYLOAD> * ) TreeItem<PAYLOAD>::_firstChild; }


private:

    /**
     * Private ( i.e. disabled ) copy constructor and operator=()
     * - neither makes any sense with this class.
     **/
    SortedTreeItem             ( const SortedTreeItem<PAYLOAD> & ) {}
    SortedTreeItem & operator= ( const SortedTreeItem<PAYLOAD> & ) {}
};



/**
 * Find a direct child ( i.e., non-recursive ) with value "searchVal".
 * Returns 0 if there is no such child.
 **/
template<class ITEM, class PAYLOAD> inline
ITEM *
findDirectChild( ITEM * item, PAYLOAD searchVal )
{
    TreeItem<PAYLOAD> * child = item->firstChild();

    while ( child )
    {
        if ( child->value() == searchVal )
            return dynamic_cast<ITEM *> ( child );

        child = child->next();
    }

    return 0;
}


#endif // TreeItem_h

/*
  Copyright (C) 2000-2012 Novell, Inc
  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/


/*-/

   File:       NCTable.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <yui/YUILog.h>
#include "NCTable.h"
#include "NCPopupMenu.h"
#include <yui/YMenuButton.h>
#include <yui/YTypes.h>

using std::endl;

NCTable::NCTable( YWidget * parent, YTableHeader *tableHeader, bool multiSelection )
    : YTable( parent, tableHeader, multiSelection )
    , NCPadWidget( parent )
    , biglist( false )
    , multiselect( multiSelection )
{
    yuiDebug() << std::endl;

    InitPad();
    // !!! head is UTF8 encoded, thus should be std::vector<NCstring>
    if ( !multiselect )
    {
	_header.assign( tableHeader->columns(), NCstring( "" ) );
	for ( int col = 0; col < tableHeader->columns(); col++ )
	{
	    if ( hasColumn( col ) )
	    {
		// set alignment first
		setAlignment( col, alignment( col ) );
		// and then append header
		_header[ col ] +=  NCstring( tableHeader->header( col ) ) ;
	    }
	}
    }
    else
    {
	_header.assign( tableHeader->columns()+1, NCstring( "" ) );

	for ( int col = 1; col <= tableHeader->columns(); col++ )
	{
	    if ( hasColumn( col-1 ) )
	    {
		// set alignment first
		setAlignment( col, alignment( col-1 ) );
		// and then append header
		_header[ col ] +=  NCstring( tableHeader->header( col-1 ) ) ;
	    }
	}
    }

    hasHeadline = myPad()->SetHeadline( _header );

}




NCTable::~NCTable()
{
    yuiDebug() << std::endl;
}



// Change individual cell of a table line (to newtext)
//		      provided for backwards compatibility

void NCTable::cellChanged( int index, int colnum, const std::string & newtext )
{
    NCTableLine * cl = myPad()->ModifyLine( index );

    if ( !cl )
    {
	yuiWarning() << "No such line: " << wpos( index, colnum ) << newtext << std::endl;
    }
    else
    {
	NCTableCol * cc = cl->GetCol( colnum );

	if ( !cc )
	{
	    yuiWarning() << "No such colnum: " << wpos( index, colnum ) << newtext << std::endl;
	}
	else
	{
	    // use NCtring to enforce recoding from 'utf8'
	    cc->SetLabel( NCstring( newtext ) );
	    DrawPad();
	}
    }
}



// Change individual cell of a table line (to newtext)

void NCTable::cellChanged( const YTableCell *cell )
{

    cellChanged( cell->itemIndex(), cell->column(), cell->label() );

}



// Set all table headers all at once

void NCTable::setHeader( std::vector<std::string> head )
{
    _header.assign( head.size(), NCstring( "" ) );
    YTableHeader *th = new YTableHeader();

    for ( unsigned int i = 0; i < head.size(); i++ )
    {
	th->addColumn( head[ i ] );
	_header[ i ] +=  NCstring( head[ i ] ) ;
    }

    hasHeadline = myPad()->SetHeadline( _header );

    YTable::setTableHeader( th );
}

//
// Return table header as std::string std::vector (alignment removed)
//
void NCTable::getHeader( std::vector<std::string> & header )
{
    header.assign( _header.size(), "" );

    for ( unsigned int i = 0; i < _header.size(); i++ )
    {
	header[ i ] =  _header[i].Str().substr( 1 ); // remove alignment
    }
}


// Set alignment of i-th table column (left, right, center).
// Create temp. header consisting of single letter;
// setHeader will append the rest.

void NCTable::setAlignment( int col, YAlignmentType al )
{
    std::string s;

    switch ( al )
    {
	case YAlignUnchanged:
	    s = 'L' ;
	    break;

	case YAlignBegin:
	    s = 'L' ;
	    break;

	case YAlignCenter:
	    s = 'C' ;
	    break;

	case YAlignEnd:
	    s = 'R' ;
	    break;
    }

    _header[ col ] = NCstring( s );
}

// Append  item (as pointed to by 'yitem')  in one-by-one
// fashion i.e. the whole table gets redrawn afterwards.
void NCTable::addItem( YItem *yitem)
{
    addItem(yitem, false); // add just this one
}

// Append item (as pointed to by 'yitem') to a table.
// This creates visual representation of new table line
// consisting of individual cells. Depending on the 2nd
// param, table is redrawn. If 'allAtOnce' is set to
// true, it is up to the caller to redraw the table.
void NCTable::addItem( YItem *yitem, bool allAtOnce )
{

    YTableItem *item = dynamic_cast<YTableItem *>( yitem );
    YUI_CHECK_PTR( item );
    YTable::addItem( item );
    unsigned int itemCount;

    if ( !multiselect )
	itemCount =  item->cellCount();
    else
	itemCount = item->cellCount()+1;

    std::vector<NCTableCol*> Items( itemCount );
    unsigned int i = 0;

    if ( !multiselect )
    {
	// Iterate over cells to create columns
	for ( YTableCellIterator it = item->cellsBegin();
	      it != item->cellsEnd();
	      ++it )
	{
            if (this->checkable(i))
            {
              Items[i] = new NCTableTag( yitem, (*it)->checked() );
              myPad()->setColumnSelection();
            }
            else
              Items[i] = new NCTableCol( NCstring(( *it )->label() ) );
	    i++;
	}
    }
    else
    {
	// Create the tag first
	Items[0] = new NCTableTag( yitem, yitem->selected() );
	i++;
	// and then iterate over cells
	for ( YTableCellIterator it = item->cellsBegin();
	      it != item->cellsEnd();
	      ++it )
	{
            if (this->checkable(i-1))
            {
              Items[i] = new NCTableTag( yitem, (*it)->checked() );
              myPad()->setColumnSelection();
            }
            else
              Items[i] = new NCTableCol( NCstring(( *it )->label() ) );
	    i++;
	}
    }

    //Insert @idx
    NCTableLine *newline = new NCTableLine( Items, item->index() );

    YUI_CHECK_PTR( newline );

    newline->setOrigItem( item );

    myPad()->Append( newline );

    if ( item->selected() )
    {
	setCurrentItem( item->index() ) ;
    }

    //in one-by-one mode, redraw the table (otherwise, leave it
    //up to the caller)
    if (!allAtOnce)
    {
	DrawPad();
    }
}

// reimplemented here to speed up item insertion
// (and prevent inefficient redrawing after every single addItem
// call)
void NCTable::addItems( const YItemCollection & itemCollection )
{

    for ( YItemConstIterator it = itemCollection.begin();
	  it != itemCollection.end();
	  ++it )
    {
	addItem( *it, true);
    }
    DrawPad();
}

// Clear the table (in terms of YTable and visually)

void NCTable::deleteAllItems()
{
    myPad()->ClearTable();
    DrawPad();
    YTable::deleteAllItems();
}



// Return index of currently selected table item

int NCTable::getCurrentItem()
{
    if ( !myPad()->Lines() )
	return -1;

    return keepSorting() ? myPad()->GetLine( myPad()->CurPos().L )->getIndex()
	   : myPad()->CurPos().L;

}



// Return origin pointer of currently selected table item

YItem * NCTable::getCurrentItemPointer()
{
    const NCTableLine *cline = myPad()->GetLine( myPad()->CurPos().L );

    if ( cline )
	return cline->origItem();
    else
	return 0;
}



// Highlight item at 'index'

void NCTable::setCurrentItem( int index )
{
    myPad()->ScrlLine( index );
}



// Mark table item (as pointed to by 'yitem') as selected

void NCTable::selectItem( YItem *yitem, bool selected )
{
    if ( ! yitem )
	return;

    YTableItem *item = dynamic_cast<YTableItem *>( yitem );
    YUI_CHECK_PTR( item );

    NCTableLine *line = ( NCTableLine * )item->data();
    YUI_CHECK_PTR( line );

    const NCTableLine *current_line = myPad()->GetLine( myPad()->CurPos().L );
    YUI_CHECK_PTR( current_line );

    if ( !multiselect && !checkable(myPad()->CurPos().C))
    {
	if ( !selected && ( line == current_line ) )
	{
	    deselectAllItems();
	}
	else
	{
	    // first highlight only, then select
	    setCurrentItem( line->getIndex() );
	    YTable::selectItem( item, selected );
	}
    }
    else if (multiselect && myPad()->CurPos().C == 0)
    {
	setCurrentItem( line->getIndex() );
	YTable::selectItem( item, selected );

	yuiMilestone() << item->label() << " is selected: " << (selected?"yes":"no") <<  endl;

	NCTableTag *tag =  static_cast<NCTableTag *>( line->GetCol( 0 ) );
	tag->SetSelected( selected );
    }

    yuiMilestone() << "(A) current line is: " << current_line->getIndex() << " selected is: " << line->getIndex() << " column pos " << myPad()->CurPos().C << endl;
    int i = (multiselect?1:0);
    // Iterate over cells to check columns
    for ( YTableCellIterator ytit = item->cellsBegin();
        /*(myPad()->CurPos().C != 0 || (myPad()->CurPos().C == 0 && !multiselect)) &&*/ ytit != item->cellsEnd();
        ++ytit )
    {
        if ((*ytit)->checkable())
        {
          if ( myPad()->CurPos().C == i)
          {
            NCTableTag *tag =  static_cast<NCTableTag *>( line->GetCol(/*multiselect ? i+1 :*/ i ) );
            tag->SetSelected( !(*ytit)->checked() );
            (*ytit)->setChecked(!(*ytit)->checked());
            //myPad()->ScrlCol(i);
            yuiMilestone() << item->label() << "(B) column selected " << (/*multiselect ? i+1 :*/ i) << " column pos " << myPad()->CurPos().C <<  endl;
          }
        }
        i++;
    }
        
    // and redraw
    DrawPad();
}



// Mark currently highlighted table item as selected
// Yeah, it is really already highlighted, so no need to
// selectItem() and setCurrentItem() here again - #493884

void NCTable::selectCurrentItem()
{
    const NCTableLine *cline = myPad()->GetLine( myPad()->CurPos().L );

    if ( cline )
	YTable::selectItem( cline->origItem(), true );
}



// Mark all items as deselected

void NCTable::deselectAllItems()
{
    yuiMilestone() << "(AAAA) column selected " <<  endl;

    setCurrentItem( -1 );
    YTable::deselectAllItems();
    DrawPad();
}



// return preferred size

int NCTable::preferredWidth()
{
    wsze sze = ( biglist ) ? myPad()->tableSize() + 2 : wGetDefsze();
    return sze.W;
}



// return preferred size

int NCTable::preferredHeight()
{
    wsze sze = ( biglist ) ? myPad()->tableSize() + 2 : wGetDefsze();
    return sze.H;
}



// Set new size of the widget

void NCTable::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}




void NCTable::setLabel( const std::string & nlabel )
{
    // not implemented: YTable::setLabel( nlabel );
    NCPadWidget::setLabel( NCstring( nlabel ) );
}



// Set widget state (enabled vs. disabled)

void NCTable::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YTable::setEnabled( do_bv );
}




bool NCTable::setItemByKey( int key )
{
    return myPad()->setItemByKey( key );
}





// Create new NCTablePad, set its background
NCPad * NCTable::CreatePad()
{
    wsze    psze( defPadSze() );
    NCPad * npad = new NCTablePad( psze.H, psze.W, *this );
    npad->bkgd( listStyle().item.plain );

    return npad;
}



// Handle 'special' keys i.e those not handled by parent NCPad class
// (space, return). Set items to selected, if appropriate.

NCursesEvent NCTable::wHandleInput( wint_t key )
{
    NCursesEvent ret;
    int citem  = getCurrentItem();

    switch (key)
    {
      case CTRL('n')://KEY_RIGHT:
      {
        yuiMilestone() << "<KEY_RIGHT> pressed on col. " << myPad()->CurPos().C <<  "/" << myPad()->Cols()<<  endl;
        const NCTableLine *cline = myPad()->GetLine( myPad()->CurPos().L );
        if ( cline )
        {
          YTableItem *item = cline->origItem(); 
          YUI_CHECK_PTR( item ); 
          int currCol = myPad()->CurPos().C;
          for (unsigned col= currCol+1; col < myPad()->Cols(); ++col)
          {
            if (this->checkable(col-(multiselect?1:0)))
            {
              yuiMilestone() << item->label() << " scroll right of " << col - currCol <<  endl;
              for (unsigned i=0; i< col - currCol; ++i)
                myPad()->ScrlRight();
              break;
            }
          }
          yuiMilestone() << item->label() << " new column selected " << myPad()->CurPos().C <<  "/" << myPad()->Cols() <<  endl;          
        }
      }
        break;
      case CTRL('p')://KEY_LEFT:
      {
        yuiMilestone() << "<KEY_LEFT> pressed " << " col. " << myPad()->CurPos().C <<  endl;
       
        const NCTableLine *cline = myPad()->GetLine( myPad()->CurPos().L );

        if ( cline )
        {
          YTableItem *item = cline->origItem(); 
          YUI_CHECK_PTR( item ); 
          
          int currCol = myPad()->CurPos().C;
          for (int col= currCol-1; col >= 0; --col)
          {
            if (col == 0)
            {
              if (multiselect || (!multiselect && this->checkable(col)))
                for (int i=0; i< currCol - col; ++i)
                  myPad()->ScrlLeft();
            }
            else if (this->checkable(col-(multiselect?1:0)))
            {
              yuiMilestone() << item->label() << " scroll left of " << currCol - col <<  endl;
              for (int i=0; i< currCol - col; ++i)
                myPad()->ScrlLeft();
              break;
            }
          }
          
          yuiMilestone() << item->label() << " new column selected " << myPad()->CurPos().C <<  " internal is " << currCol <<  endl;          
        }
      }
        break;
        case CTRL( 'o' ):
                {
                    if ( ! keepSorting() )
                    {
                        // get the column
                        wpos at( ScreenPos() + wpos( win->height() / 2, 1 ) );

                        YItemCollection ic;
                        ic.reserve( _header.size() );
                        unsigned int i = 0;

                        for ( std::vector<NCstring>::const_iterator it = _header.begin();
                              it != _header.end() ; it++, i++ )
                        {
                            // strip the align mark
                            std::string col = ( *it ).Str();
                            col.erase( 0, 1 );

                            YMenuItem *item = new YMenuItem( col ) ;
                            //need to set index explicitly, MenuItem inherits from TreeItem
                            //and these don't have indexes set
                            item->setIndex( i );
                            ic.push_back( item );
                        }

                        NCPopupMenu *dialog = new NCPopupMenu( at, ic.begin(), ic.end() );

                        int column = dialog->post();

                        if ( column != -1 )
                            myPad()->setOrder( column, true );  //enable sorting in reverse order

                        //remove the popup
                        YDialog::deleteTopmostDialog();

                        return NCursesEvent::none;
                    }
                }

            case KEY_SPACE:
            case KEY_RETURN:
              yuiMilestone() << (key == KEY_SPACE ? "<SPACE> " : "<RETURN> ") << "pressed(1) col. " << myPad()->CurPos().C << " checkable "<< (this->checkable(myPad()->CurPos().C) ? "yes " : "no") <<  endl;

                if ( !multiselect )
                {
                  if (this->checkable(myPad()->CurPos().C))
                  {  
                      toggleCurrentItem();
                      if ( notify() && citem != -1 )
                              return NCursesEvent::ValueChanged;
                  }

                  if ( notify() && citem != -1 )
                      return NCursesEvent::Activated;
                }
                else
                {
                      toggleCurrentItem();
                }
                yuiMilestone() << (key == KEY_SPACE ? "<SPACE> " : "<RETURN> ") << "pressed(2) col. " << myPad()->CurPos().C << " checkable "<< (this->checkable(myPad()->CurPos().C) ? "yes " : "no") <<  endl;
                break;
            default:
              handleInput( key );
              break;
    }
#if 0    
    if ( ! handleInput( key ) )
    {
	switch ( key )
	{
	    case CTRL( 'o' ):
		{
		    if ( ! keepSorting() )
		    {
			// get the column
			wpos at( ScreenPos() + wpos( win->height() / 2, 1 ) );

			YItemCollection ic;
			ic.reserve( _header.size() );
			unsigned int i = 0;

			for ( std::vector<NCstring>::const_iterator it = _header.begin();
			      it != _header.end() ; it++, i++ )
			{
			    // strip the align mark
			    std::string col = ( *it ).Str();
			    col.erase( 0, 1 );

			    YMenuItem *item = new YMenuItem( col ) ;
			    //need to set index explicitly, MenuItem inherits from TreeItem
			    //and these don't have indexes set
			    item->setIndex( i );
			    ic.push_back( item );
			}

			NCPopupMenu *dialog = new NCPopupMenu( at, ic.begin(), ic.end() );

			int column = dialog->post();

			if ( column != -1 )
			    myPad()->setOrder( column, true );	//enable sorting in reverse order

			//remove the popup
			YDialog::deleteTopmostDialog();

			return NCursesEvent::none;
		    }
		}

	    case KEY_SPACE:
              yuiMilestone() << "<SPACE> pressed col. " << myPad()->CurPos().C <<  endl;
	    case KEY_RETURN:
              yuiMilestone() << "<RETURN> pressed col. " << myPad()->CurPos().C <<  endl;
		if ( !multiselect )
		{
		    if ( notify() && citem != -1 )
			return NCursesEvent::Activated;
		}
// 		else
// 		{
		    toggleCurrentItem();
// 		}
		break;
            case KEY_STAB:
              yuiMilestone() << "<TAB> pressed col. " << myPad()->CurPos().C <<  endl;

              break;
            default:
             yuiMilestone() << "Key pressed " << key << " col. " << myPad()->CurPos().C <<  endl;

              break;
	}
    }
#endif

    if (  citem != getCurrentItem() )
    {
	if ( notify() && immediateMode() )
	    ret = NCursesEvent::SelectionChanged;

	if ( !multiselect )
	    selectCurrentItem();
    }

    return ret;
}


/**
 * Toggle item from selected -> deselected and vice versa
 **/
void NCTable::toggleCurrentItem()
{
    YTableItem *it =  dynamic_cast<YTableItem *>( getCurrentItemPointer() );
    if ( it )
    {  
	selectItem( it, !( it->selected() ) );
    }
}

import { Component, OnInit, Input } from '@angular/core';
import { Bookshelf } from '../bookshelf';
import { BookshelfService } from '../bookshelf.service';

@Component({
  selector: 'app-bookshelf',
  templateUrl: './bookshelf.component.html',
  styleUrls: ['./bookshelf.component.scss']
})
export class BookshelfComponent implements OnInit {

  constructor(private bookshelfService: BookshelfService) { }

  ngOnInit() {
    this.getBookshelf();
  }

  getBookshelf() : void {
    this.bookshelfService.getBookshelf().
      subscribe(bookshelf => this.bookshelves = bookshelf);
  }
 
  onClickMe(shelf: Bookshelf, on:number) {
    shelf.on = (on === 1);
    this.bookshelfService.updateBookshelf(shelf).
      subscribe(bookshelf => this.bookshelves = bookshelf);
  }

  bookshelves: Bookshelf[];
}

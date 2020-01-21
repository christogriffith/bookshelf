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
      subscribe(bookshelf => this.bookshelf = bookshelf);
  }
 
  onClickMe(shelf: Bookshelf, on:string) {
    shelf.power = on;
    this.bookshelfService.updateBookshelf(shelf).
      subscribe(bookshelf => this.bookshelf = bookshelf);
  }

  onColorChange(shelf: Bookshelf, color: string)
  {
    console.log("changed color to" + color);
    shelf.cmd="entire";
    var Color = require('color');
    var col = Color(color);
    shelf.color = col.object();
    console.log(shelf);
    this.bookshelfService.updateBookshelf(shelf).
      subscribe(bookshelf => this.bookshelf = bookshelf);
  }

  bookshelf: Bookshelf;
}

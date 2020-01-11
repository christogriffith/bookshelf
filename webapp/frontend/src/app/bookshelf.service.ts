import { Injectable } from '@angular/core';

import { Bookshelf } from './bookshelf';

// Fake data, get this from python
const BOOKSHELVES: Bookshelf[] = [ 
  { id: 0, on: false },
  { id: 1, on: false }
];

@Injectable({
  providedIn: 'root'
})
export class BookshelfService {

  constructor() { }

  getBookshelf() {
    return BOOKSHELVES;
  }

}


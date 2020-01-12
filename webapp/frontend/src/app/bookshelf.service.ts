import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders } from '@angular/common/http';

import { Observable, of } from 'rxjs';
import { catchError, map, tap } from 'rxjs/operators';

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

  private bookshelfUrl = 'http://bookshelf:5002';

  constructor(
    private http: HttpClient) { }

  getBookshelf(): Observable<Bookshelf[]> {
    return this.http.get<Bookshelf[]>(this.bookshelfUrl);
  }

  updateBookshelf (bookshelf: Bookshelf): Observable<any> {
    const url = `${this.bookshelfUrl}/${bookshelf.id}`;
    let param = 0;  
    if (bookshelf.on)
      param = 1;
    return this.http.put(url, `on=${param}`);
  }   


}


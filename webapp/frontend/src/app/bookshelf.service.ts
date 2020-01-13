import { Injectable } from '@angular/core';
import { HttpClient, HttpHeaders } from '@angular/common/http';

import { Observable, of } from 'rxjs';
import { catchError, map, tap } from 'rxjs/operators';

import { Bookshelf } from './bookshelf';

@Injectable({
  providedIn: 'root'
})
export class BookshelfService {

  private bookshelfUrl = 'http://bookshelf:5002';

  constructor(
    private http: HttpClient) { }

  getBookshelf(): Observable<Bookshelf> {
    return this.http.get<Bookshelf>(this.bookshelfUrl);
  }

  updateBookshelf (bookshelf: Bookshelf): Observable<any> {
    
    const url = `${this.bookshelfUrl}`;
    let param = 0;  
    if (bookshelf.power == "on")
      param = 1;
    let ret = this.http.put(url, bookshelf);
    return ret;
  }   


}


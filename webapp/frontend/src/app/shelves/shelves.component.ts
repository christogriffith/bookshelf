import { Component, OnInit } from '@angular/core';
import { Shelf } from '../shelf';

import { SHELVES } from '../def-shelves'

@Component({
  selector: 'app-shelves',
  templateUrl: './shelves.component.html',
  styleUrls: ['./shelves.component.scss']
})
export class ShelvesComponent implements OnInit {

  constructor() { }

  ngOnInit() {
  }

  /* TODO: should probably capitalize the shelf type */
  selectedShelf: Shelf;
  onselect(shelf: Shelf): void {
    this.selectedShelf = shelf;
  }
  
  shelves = SHELVES;

}


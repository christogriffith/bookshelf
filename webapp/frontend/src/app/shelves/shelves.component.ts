import { Component, OnInit } from '@angular/core';
import { Shelf } from '../shelf';

import SHELVES from '../def-shelves.'

@Component({
  selector: 'app-shelves',
  templateUrl: './shelves.component.html',
  styleUrls: ['./shelves.component.scss']
})
export class ShelvesComponent implements OnInit {

  shelf: Shelf = {
    id: 1,
    name: "Left0"
  };

  constructor() { }

  ngOnInit() {
  }
  
  shelves = SHELVES;

}

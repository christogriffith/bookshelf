import { Component, OnInit, Input } from '@angular/core';

import { Shelf } from '../shelf';

@Component({
  selector: 'app-shelf-detail',
  templateUrl: './shelf-detail.component.html',
  styleUrls: ['./shelf-detail.component.scss']
})

export class ShelfDetailComponent implements OnInit {
  @Input() shelf: Shelf;

  constructor() { }

  ngOnInit() {
  }

}

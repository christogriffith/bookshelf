import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { FormsModule } from '@angular/forms';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { ShelvesComponent } from './shelves/shelves.component';
import { ShelfDetailComponent } from './shelf-detail/shelf-detail.component';

@NgModule({
  declarations: [
    AppComponent,
    ShelvesComponent,
    ShelfDetailComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    FormsModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }

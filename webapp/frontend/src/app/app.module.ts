import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { FormsModule } from '@angular/forms';
import { HttpClientModule } from '@angular/common/http';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { ShelvesComponent } from './shelves/shelves.component';
import { ShelfDetailComponent } from './shelf-detail/shelf-detail.component';
import { BookshelfComponent } from './bookshelf/bookshelf.component';
import { LedGroupOnecolorComponent } from './led-group-onecolor/led-group-onecolor.component';

@NgModule({
  declarations: [
    AppComponent,
    ShelvesComponent,
    ShelfDetailComponent,
    BookshelfComponent,
    LedGroupOnecolorComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    FormsModule,
    HttpClientModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }

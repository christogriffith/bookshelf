import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LedGroupOnecolorComponent } from './led-group-onecolor.component';

describe('LedGroupOnecolorComponent', () => {
  let component: LedGroupOnecolorComponent;
  let fixture: ComponentFixture<LedGroupOnecolorComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ LedGroupOnecolorComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LedGroupOnecolorComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});

#ifndef THEME_H
#define THEME_H

// Applies (or removes) the app's dark mode stylesheet on the whole
// application. Deliberately kept as a couple of free functions rather
// than a class, there's no state to own here, QApplication itself
// already holds the current stylesheet.
namespace Theme {

// Call once, right after QApplication is constructed and before any
// windows are shown, so the very first thing the user sees (the
// login screen) already reflects their saved preference.
void apply(bool darkModeEnabled);

} // namespace Theme

#endif // THEME_H

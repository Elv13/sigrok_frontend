# Extra Qt model proxies

This library is a collection of work-in-progress proxies to be used on
Qt `QAbstractItemModel` objects. Those proxies were written in their own project
context. While there was some effort to make the code as generic and re-usable
as possible, **NONE** of them are implemented beyond what was necessary to be
useful for their purpose. This, in turn means any project wanting to use them
will probably have to implement some of the missing features, such as support
for the `rowsRemoved` and `rowsMoved` events.

## License

This project is available either as GPLv3 + copyright assignment or with the
Qt commercial license for a fee.

Some proxies are also available with an MIT licence:

 * QTypeColoriserProxy
 * QObjectModel
 * QReactiveProxyModel
 * QMultiModelTree

## Status

### No known bugs

 * LastRowToListProxy
 * FlagsFilterProxy
 * FilterTopLevelProxy

### Working for simple use cases

The "simple" use case is when a model only has static content or append new rows
one at the time and never remove them.

### Will requires further API changes or major refactoring

 * RangeProxy
 * ColoredRangeProxy

### Wont ever support all model topologies

 * QMultiModelTree: It isn't possible to support trees, even with access to Qt
   internals.

### Broken

 * HeadProxy: Easy to fix, but I have no use for it

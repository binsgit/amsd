# ChangeLog
All notable changes to this project will be documented in this file.

## [Unreleased] - 2017-04-24

### Changed
- Program structure reconstruct:
  * Functions are re-arranged by namespaces and classes.
  * Unified error handling.
- Better config file parsing mechanism.
- Some commonly-used functions are moved to codebase [libReimu](https://github.com/CloudyReimu/libReimu) for better robustness:
  * Network address manipulations
  * Automated database operations
  * Filesystem operations
- Complete rewrite of Database part:
  * Parts that are not performance-critical are OOPed for easy use.
  * Commonly-used SQL statements are auto-generated.
  * Columns can be constructed by an initialize list instead of a looooong string.
  * Data type of columns can be retrieved and tested by bitwise operations.
  * Bind() and Column() takes value of **any** data type. e.g. `Bind(1, 3.14)` and `Bind(2, "hello")`
- API string from CgMiner are parsed in a more efficient way instead of regexp.


## [0.3] - 2017-03-22
### Changed
- Uses shared memory built on file-based backing storage as NVRAM for time-related synchronisations.
- Last data collection time is stored separately to decrease the pressure of database.
- Removed global lock. All database operations can run in parallel without collisions.

### Removed
- MMUpgrade. Please use the universal SuperRTAC instead.
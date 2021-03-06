/*
 * Copyright (c) 2001, 2003 Robert Collins.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     A copy of the GNU General Public License can be found at
 *     http://www.gnu.org/
 *
 * Written by Robert Collins  <rbtcollins@hotmail.com>
 *
 */

#ifndef SETUP_PACKAGE_META_H
#define SETUP_PACKAGE_META_H

class SolvableVersion;
typedef SolvableVersion packageversion;
class packagemeta;

#include <set>
#include <vector>
#include "PackageTrust.h"
#include "package_version.h"
#include "package_message.h"
#include "script.h"
#include "ActionList.h"

typedef std::pair<const std::string, std::vector<packagemeta *> > Category;

/* NOTE: A packagemeta without 1 version is invalid! */
class packagemeta
{
public:
  static void ScanDownloadedFiles (bool);
  packagemeta (packagemeta const &);
  packagemeta (const std::string& pkgname)
    : name (pkgname), user_picked (false),
      _action(NoChange_action), _picked(false), _srcpicked(false)
  {
  }

  ~packagemeta ();

  SolvableVersion add_version (const SolverPool::addPackageData &);
  void set_installed_version (const std::string &);
  void addToCategoryBase();
  bool hasNoCategories() const;
  void setDefaultCategories();
  void addToCategoryAll();

  enum _actions
    {
     NoChange_action = 1,  // keep if installed, skip if not installed
     Install_action,
     Reinstall_action,
     Uninstall_action,
    };
  static const char *action_caption (_actions value);

  void set_action (_actions, packageversion const & default_version,
                   bool useraction = false);
  ActionList *list_actions(trusts const trust);
  void select_action (int id, trusts const deftrust);
  void toggle_action ();
  _actions get_action () { return _action; }

  void set_message (const std::string& message_id, const std::string& message_string)
  {
    message.set (message_id, message_string);
  }

  void set_version_blacklist(std::set <std::string> &_list)
  {
    version_blacklist = _list;
  }

  std::string action_caption () const;
  packageversion trustp(bool _default, trusts const t) const 
  {
    /* If the user chose "test" and a "test" version is available, return it. */
    if (t == TRUST_TEST && exp) return exp;
    /* Are we looking for the default version and does the installed version
       have a higher version number than the "curr" package?  This means the
       user has installed a "test" version, or built her own version newer
       than "curr".  Rather than pulling the user back to "curr", we install
       "test" if a "test" version is available and the version number is higher,
       or we stick to "installed" if not.  This reflects the behaviour of
       `yum update' on Fedora. */
    if (_default && curr && installed &&
        packageversion::compareVersions(curr, installed) < 0) {
      if (exp && packageversion::compareVersions(installed, exp) < 0)
        return exp;
      return installed;
    }
    /* Otherwise, if a "curr" version exists, return "curr". */
    if (curr) return curr;
    /* Otherwise return the installed version. */
    return installed;
  }

  std::string name;			/* package name, like "cygwin" */

  /* true if package was selected on command-line. */
  bool isManuallyWanted() const;
  /* true if package was deleted on command-line. */
  bool isManuallyDeleted() const;

  const std::string SDesc () const;
  const std::string LDesc () const;

  /* what categories does this package belong in. Note that if multiple versions
   * of a package disagree.... the first one read in will take precedence.
   */
  void add_category (const std::string& );
  std::set <std::string, casecompare_lt_op> categories;
  const std::string getReadableCategoryList () const;
  std::set <packageversion> versions;

  /* Did the user already pick a version at least once? */
  bool user_picked;
  /* which one is installed. */
  packageversion installed;
  /* which one is listed as "current" (stable) in our available packages db */
  packageversion curr;
  /* ditto for "test" (experimental) */
  packageversion exp;
  /* which one is the default according to the solver */
  packageversion default_version;
  /* Now for the user stuff :] */
  /* What version does the user want ? */
  packageversion desired;

  bool picked() const;   /* true if desired version is to be (re-)installed */
  void pick(bool); /* trigger an install/reinstall */

  bool srcpicked() const;   /* true if source for desired version is to be installed */
  void srcpick(bool);

  /* can one or more versions be installed? */
  bool accessible () const;
  bool sourceAccessible() const;

  bool isBinary() const;

  void logSelectionStatus() const;
  void logAllVersions() const;

  void addScript(Script const &);
  std::vector <Script> &scripts();

  /* this version is undesirable */
  bool isBlacklisted(const packageversion &version) const;

protected:
  packagemeta &operator= (packagemeta const &);
private:
  std::string trustLabel(packageversion const &) const;
  std::vector <Script> scripts_;
  static bool scan (const packageversion &pkg, bool mirror_mode);

  _actions _action;

  bool _picked; /* true if desired version is to be (re)installed */
  bool _srcpicked;

  packagemessage message;
  std::set <std::string> version_blacklist;
};

#endif /* SETUP_PACKAGE_META_H */

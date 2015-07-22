/* 
 * 
 * Author: Gediminas Dulskas <dulskasg@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "includes.h"
#include "settings.h"
#include "structs.h"
#ifndef UTILS_H
#define	UTILS_H

namespace utils {
    
    /* Key */
    is import_key_from_keyserver(string homedir, string keyserver, string key);
    vector<ks> search_keyserver(string homedir, string keyserver, string key);
    
    /* Gpgme */
    void init_gpgme(gpgme_ctx_t *ctx, string gpgdir);
    
    /* Settings*/
    void read_settings(string sett, gpgme_ctx_t ctx, gpgme_key_t *the_key, string *alias, string *fingerprint);
    
    /* Tracker queue*/
    vector<tqs> get_tq(int action);
    void insert_tq(tqs tq);
    void remove_tq(int id);
    bool update_tq(string transaction_id, vector<ss> scrape_queue);
    bool update_tq(string transaction_id, vector<ads> announce_queue, int seeders, int leechers);
    
    /* Tests */
    void test_import(string homedir, string keyserver, string key);
    void test_search(string homedir, string keyserver, string key);
    
}
#endif	/* UTILS_H */


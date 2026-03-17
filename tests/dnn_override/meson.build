# Copyright (C) 2026 by Keith Milner <kamilner@sslconsult.com>

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

test_dnn_override_src = [
  'test_dnn_override.c',
]

test_dnn_override_exe = executable('test-dnn-override',
  test_dnn_override_src,
  include_directories : ['../../lib', '../../src', testinc],
  dependencies : [ libapp_dep, libsmf_dep, libcore_dep, libgtp_dep, libpfcp_dep, libsbi_dep ],
)

test('dnn_override', test_dnn_override_exe, suite: 'smf')

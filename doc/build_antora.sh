#
# Copyright (c) 2023 Alan de Freitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/openmethod
#

set -e

if [ $# -eq 0 ]
  then
    echo "No playbook supplied, using default playbook"
    PLAYBOOK="local-playbook.yml"
  else
    PLAYBOOK=$1
fi

echo "Building documentation with Antora..."
echo "Installing npm dependencies..."
npm ci

echo "Building docs in custom dir..."
PATH="$(pwd)/node_modules/.bin:${PATH}"
export PATH
npx antora --clean --fetch "$PLAYBOOK" --stacktrace --log-level all

echo "Fixing links to non-mrdocs URIs..."

for f in $(find build/site -name '*.html'); do
  perl -i -pe 's{&lsqb;(.*?)&rsqb;\(([^)]+)\)}{<a href="../../$2">$1</a>}g' "$f"
done

echo "Done"

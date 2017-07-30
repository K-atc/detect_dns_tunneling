#!/bin/sh

usage(){
    echo "usage: $0 MODE"
    echo "MODE = BASE_N_ENCODE, HEX_ENCODE"
    exit
}

if [ $# -eq 0 ]; then
    usage
fi

MODE=$1

gcc -o detect_dns_tunneling detect_dns_tunneling.c -lm -D"$MODE" || exit

rm classifier_result*.csv

if [ ! -e data/clean-fqdn.txt ]; then
    for file in `ls ../../DNS/subdomains/*.subdomains.txt`; do cat $file && echo ""; done | egrep -v ^$ > data/clean-fqdn.txt || exit
fi

echo "[*] === [Clean FQDN] ==="
./detect_dns_tunneling data/clean-fqdn.txt Y || exit

if [ $MODE = "BASE_N_ENCODE" ]; then
    echo "[*] === [DNS Tunneling] ==="
    if [ ! -e data/leaked.pdf ]; then
        echo "[*] Downloading PDF..."
        curl -sk http://www.seisakukikaku.metro.tokyo.jp/tokyo_vision/vision_index/pdf/honbun2_3.pdf -o data/leaked.pdf
    fi
    cat data/leaked.pdf | base64 -w 20 | head -n 50000 | sed -e 's/$/.campaign.evil.jp/' > data/dns-tunneling-base64.txt
    cat data/leaked.pdf | base32 -w 20 | head -n 50000 | sed -e 's/$/.campaign.evil.jp/' > data/dns-tunneling-base32.txt
    echo "[*] -- base64"
    ./detect_dns_tunneling data/dns-tunneling-base64.txt N || exit
    echo "[*] -- base32"
    ./detect_dns_tunneling data/dns-tunneling-base32.txt N || exit
fi

if [ $MODE = "HEX_ENCODE" ]; then
    echo "[*] === [hex encoding] ==="
    if [ ! -e data/cano.txt ]; then
        echo "[*] Downloading text..."
        curl -k https://sherlock-holm.es/stories/plain-text/cano.txt -o data/cano.txt
    fi
    cat data/cano.txt | fold -s10 | od -A n -t x1 | tr -d ' ' | sed -e 's/$/.campaign.evil.jp/' | head -n 100000 > data/dns-tunneling-hex.txt
    ./detect_dns_tunneling data/dns-tunneling-hex.txt N || exit
fi

echo "[*] Plot ROC"
python plot_roc_chart.py $MODE

# cat *.subdomains.txt | sed -e 's/\.[^..]*[\.][^..]*$//' | sort | uniq > ~/Dropbox/Project/C92/yara-pcap-test/detect_dns_tunneling/clean-subdomain.txt 
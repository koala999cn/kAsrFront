compute-mfcc-feats --use-energy=false --cepstral-lifter=0.0 --dither=0  --preemphasis-coefficient=0 --energy-floor=1 --raw-energy=true --remove-dc-offset=false --round-to-power-of-two=false --snip-edges=true  --window-type="rectangular" scp,p:test.scp ark,t:mfcc-plain.txt
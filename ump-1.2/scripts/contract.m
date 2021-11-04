%contract.m:  generate r-way cut from previous cut
function [Nodenew, Enew] = contract(Node, E);

% Sum of all edges
S=sum(sum(E));
[n,m]=size(E);

% Random number in S
x = S*rand;

% Loop to find minimal l such that sum >= x
cum=0; l0=0;
while cum < x,
  l0 = l0 + 1;
  indexi = floor((l0-1)/n)+1;
  indexj = l0-floor((l0-1)/n)*n;
  cum = cum + E(indexi, indexj);
end


% Find nodes connected by l0
node_alpha  = Node(indexi);
node_beta  =  Node(indexj);
merged_node_num = min(node_alpha, node_beta);
test_node_num = max(node_alpha, node_beta);
for inode=1:length(Node),
   if Node(inode) == test_node_num,
      Node(inode) =  merged_node_num;
   end
end


% Remove all self loops
for jnode=1:m,
  for inode=jnode+1:n,
      if Node(inode) == Node(jnode),
         E(jnode,inode)=0;
      end
   end
end
   


% Return
Nodenew=Node;
Enew=E;




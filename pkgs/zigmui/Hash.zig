// 32bit fnv-1a hash
const HASH_INITIAL = 2166136261;

fn hash(src: Id, data: []const u8) Id {
  var p = &data[0];
  var size = data.len;
  var hash = src;
  while (size>=0) : ({size-=1; p+=1;}) {
    hash = (hash ^ p.*) * 16777619;
  }
  return hash;
}

fn get_id(self: *Self, data:[]const u8) Id
{
  const res = if(ctx.id_stack.back)|val| val else HASH_INITIAL;
  self.last_id = hash(res, data, size);
  return self.last_id;
}
